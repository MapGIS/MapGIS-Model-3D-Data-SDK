import { CesiumZondy } from '../core/Base';
import { Zlib } from '../thirdParty/inflate.min';
import MapGISM3DDataContent from './MapGISM3DDataContent';

const { deprecationWarning } = Cesium;

const scratchCartesian = new Cesium.Cartesian3();

function isPriorityDeferred(tileParam, frameStateParam) {
    const tile = tileParam;
    const frameState = frameStateParam;
    const tileset = tile._tileset;

    // If closest point on line is inside the sphere then set foveatedFactor to 0. Otherwise, the dot product is with the line from camera to the point on the sphere that is closest to the line.
    const { camera } = frameState;
    const { boundingSphere } = tile;
    const { radius } = boundingSphere;
    const scaledCameraDirection = Cesium.Cartesian3.multiplyByScalar(
        camera.directionWC,
        tile._centerZDepth,
        scratchCartesian
    );

    const closestPointOnLine = Cesium.Cartesian3.add(camera.positionWC, scaledCameraDirection, scratchCartesian);
    // The distance from the camera's view direction to the tile.
    const toLine = Cesium.Cartesian3.subtract(closestPointOnLine, boundingSphere.center, scratchCartesian);
    const distanceToCenterLine = Cesium.Cartesian3.magnitude(toLine);
    const notTouchingSphere = distanceToCenterLine > radius;

    // If camera's direction vector is inside the bounding sphere then consider
    // this tile right along the line of sight and set _foveatedFactor to 0.
    // Otherwise,_foveatedFactor is one minus the dot product of the camera's direction
    // and the vector between the camera and the point on the bounding sphere closest to the view line.
    if (notTouchingSphere) {
        const toLineNormalized = Cesium.Cartesian3.normalize(toLine, scratchCartesian);
        const scaledToLine = Cesium.Cartesian3.multiplyByScalar(toLineNormalized, radius, scratchCartesian);
        const closestOnSphere = Cesium.Cartesian3.add(boundingSphere.center, scaledToLine, scratchCartesian);
        const toClosestOnSphere = Cesium.Cartesian3.subtract(closestOnSphere, camera.positionWC, scratchCartesian);
        const toClosestOnSphereNormalize = Cesium.Cartesian3.normalize(toClosestOnSphere, scratchCartesian);
        tile._foveatedFactor = 1.0 - Math.abs(Cesium.Cartesian3.dot(camera.directionWC, toClosestOnSphereNormalize));
    } else {
        tile._foveatedFactor = 0.0;
    }

    // Skip this feature if: non-skipLevelOfDetail and replace refine, if the foveated settings are turned off, if tile is progressive resolution and replace refine and skipLevelOfDetail (will help get rid of ancestor artifacts faster)
    // Or if the tile is a preload of any kind
    const replace = tile.refine === Cesium.Cesium3DTileRefine.REPLACE;
    const skipLevelOfDetail = tileset._skipLevelOfDetail;
    if (
        (replace && !skipLevelOfDetail) ||
        !tileset.foveatedScreenSpaceError ||
        tileset.foveatedConeSize === 1.0 ||
        (tile._priorityProgressiveResolution && replace && skipLevelOfDetail) ||
        tileset._pass === Cesium.Cesium3DTilePass.PRELOAD_FLIGHT ||
        tileset._pass === Cesium.Cesium3DTilePass.PRELOAD
    ) {
        return false;
    }

    const maximumFovatedFactor = 1.0 - Math.cos(camera.frustum.fov * 0.5); // 0.14 for fov = 60. NOTE very hard to defer vertically foveated tiles since max is based on fovy (which is fov). Lowering the 0.5 to a smaller fraction of the screen height will start to defer vertically foveated tiles.
    const foveatedConeFactor = tileset.foveatedConeSize * maximumFovatedFactor;

    // If it's inside the user-defined view cone, then it should not be deferred.
    if (tile._foveatedFactor <= foveatedConeFactor) {
        return false;
    }

    // Relax SSE based on how big the angle is between the tile and the edge of the foveated cone.
    const range = maximumFovatedFactor - foveatedConeFactor;
    const normalizedFoveatedFactor = Cesium.Math.clamp((tile._foveatedFactor - foveatedConeFactor) / range, 0.0, 1.0);
    const sseRelaxation = tileset.foveatedInterpolationCallback(
        tileset.foveatedMinimumScreenSpaceErrorRelaxation,
        tileset.maximumScreenSpaceError,
        normalizedFoveatedFactor
    );
    const sse =
        tile._screenSpaceError === 0.0 && Cesium.defined(tile.parent)
            ? tile.parent._screenSpaceError * 0.5
            : tile._screenSpaceError;

    return tileset.maximumScreenSpaceError - sseRelaxation <= sse;
}

const scratchJulianDate = new Cesium.JulianDate();

function isPriorityProgressiveResolution(tilesetParam, tileParam) {
    const tileset = tilesetParam;
    const tile = tileParam;
    if (tileset.progressiveResolutionHeightFraction <= 0.0 || tileset.progressiveResolutionHeightFraction > 0.5) {
        return false;
    }

    let isProgressiveResolutionTile = tile._screenSpaceErrorProgressiveResolution > tileset._maximumScreenSpaceError; // Mark non-SSE leaves
    tile._priorityProgressiveResolutionScreenSpaceErrorLeaf = false; // Needed for skipLOD
    const { parent } = tile;
    const maximumScreenSpaceError = tileset._maximumScreenSpaceError;
    const tilePasses = tile._screenSpaceErrorProgressiveResolution <= maximumScreenSpaceError;
    const parentFails =
        Cesium.defined(parent) && parent._screenSpaceErrorProgressiveResolution > maximumScreenSpaceError;
    if (tilePasses && parentFails) {
        // A progressive resolution SSE leaf, promote its priority as well
        tile._priorityProgressiveResolutionScreenSpaceErrorLeaf = true;
        isProgressiveResolutionTile = true;
    }
    return isProgressiveResolutionTile;
}

function getPriorityReverseScreenSpaceError(tilesetParam, tileParam) {
    const tileset = tilesetParam;
    const tile = tileParam;
    const { parent } = tile;
    const useParentScreenSpaceError =
        Cesium.defined(parent) &&
        (!tileset._skipLevelOfDetail || tile._screenSpaceError === 0.0 || parent.hasTilesetContent);
    const screenSpaceError = useParentScreenSpaceError ? parent._screenSpaceError : tile._screenSpaceError;
    return tileset.root._screenSpaceError - screenSpaceError;
}

function updateExpireDate(tileParam) {
    const tile = tileParam;
    if (Cesium.defined(tile.expireDuration)) {
        const expireDurationDate = Cesium.JulianDate.now(scratchJulianDate);
        Cesium.JulianDate.addSeconds(expireDurationDate, tile.expireDuration, expireDurationDate);

        if (Cesium.defined(tile.expireDate)) {
            if (Cesium.JulianDate.lessThan(tile.expireDate, expireDurationDate)) {
                Cesium.JulianDate.clone(expireDurationDate, tile.expireDate);
            }
        } else {
            tile.expireDate = Cesium.JulianDate.clone(expireDurationDate);
        }
    }
}

function getContentFailedFunction(tileParam) {
    const tile = tileParam;

    return (error) => {
        tile._contentState = Cesium.Cesium3DTileContentState.FAILED;
        tile._contentReadyPromise.reject(error);
        tile._contentReadyToProcessPromise.reject(error);
    };
}

function createPriorityFunction(tile) {
    return () => {
        return tile._priority;
    };
}

const scratchProjectedBoundingSphere = new Cesium.BoundingSphere();

function getBoundingVolume(tileParam, frameStateParam) {
    const tile = tileParam;
    const frameState = frameStateParam;
    if (frameState.mode !== Cesium.SceneMode.SCENE3D && !Cesium.defined(tile._boundingVolume2D)) {
        const { boundingSphere } = tile._boundingVolume;
        const sphere = Cesium.BoundingSphere.projectTo2D(
            boundingSphere,
            frameState.mapProjection,
            scratchProjectedBoundingSphere
        );
        tile._boundingVolume2D = new Cesium.TileBoundingSphere(sphere.center, sphere.radius);
    }

    return frameState.mode !== Cesium.SceneMode.SCENE3D ? tile._boundingVolume2D : tile._boundingVolume;
}

function getContentBoundingVolume(tileParam, frameStateParam) {
    const tile = tileParam;
    const frameState = frameStateParam;
    if (frameState.mode !== Cesium.SceneMode.SCENE3D && !Cesium.defined(tile._contentBoundingVolume2D)) {
        const { boundingSphere } = tile._contentBoundingVolume;
        const sphere = Cesium.BoundingSphere.projectTo2D(
            boundingSphere,
            frameState.mapProjection,
            scratchProjectedBoundingSphere
        );
        tile._contentBoundingVolume2D = new Cesium.TileBoundingSphere(sphere.center, sphere.radius);
    }
    return frameState.mode !== Cesium.SceneMode.SCENE3D ? tile._contentBoundingVolume2D : tile._contentBoundingVolume;
}

const scratchToTileCenter = new Cesium.Cartesian3();

const scratchMatrix = new Cesium.Matrix3();
const scratchScale = new Cesium.Cartesian3();
const scratchHalfAxes = new Cesium.Matrix3();
const scratchCenter = new Cesium.Cartesian3();
const scratchRectangle = new Cesium.Rectangle();
const scratchOrientedBoundingBox = new Cesium.OrientedBoundingBox();
const scratchTransform = new Cesium.Matrix4();

function createBox(box, transform, result) {
    let center = Cesium.Cartesian3.fromElements(box[0], box[1], box[2], scratchCenter);
    let halfAxes = Cesium.Matrix3.fromArray(box, 3, scratchHalfAxes);

    // Find the transformed center and halfAxes
    center = Cesium.Matrix4.multiplyByPoint(transform, center, center);
    // let rotationScale = Cesium.Matrix4.getRotation(transform, scratchMatrix);

    let rotationScale;
    // qwk 此处新版本的接口发生了变动
    if (Cesium.defined(Cesium.Matrix4.getRotation)) {
        rotationScale = Cesium.Matrix4.getRotation(transform, scratchMatrix);
    } else {
        rotationScale = Cesium.Matrix4.getMatrix3(transform, scratchMatrix);
    }

    halfAxes = Cesium.Matrix3.multiply(rotationScale, halfAxes, halfAxes);

    if (Cesium.defined(result)) {
        result.update(center, halfAxes);
        return result;
    }
    return new Cesium.TileOrientedBoundingBox(center, halfAxes);
}

function createBoxFromTransformedRegion(region, transformParam, initialTransform, result) {
    let transform = transformParam;
    const rectangle = Cesium.Rectangle.unpack(region, 0, scratchRectangle);
    const minimumHeight = region[4];
    const maximumHeight = region[5];

    const orientedBoundingBox = Cesium.OrientedBoundingBox.fromRectangle(
        rectangle,
        minimumHeight,
        maximumHeight,
        Cesium.Ellipsoid.WGS84,
        scratchOrientedBoundingBox
    );
    let { center } = orientedBoundingBox;
    let { halfAxes } = orientedBoundingBox;

    // A region bounding volume is not transformed by the transform in the tileset JSON,
    // but may be transformed by additional transforms applied in Cesium.
    // This is why the transform is calculated as the difference between the initial transform and the current transform.
    transform = Cesium.Matrix4.multiplyTransformation(
        transform,
        Cesium.Matrix4.inverseTransformation(initialTransform, scratchTransform),
        scratchTransform
    );
    center = Cesium.Matrix4.multiplyByPoint(transform, center, center);

    // let rotationScale = Cesium.Matrix4.getRotation(transform, scratchMatrix);
    let rotationScale;
    // qwk 此处新版本的接口发生了变动
    if (Cesium.defined(Cesium.Matrix4.getRotation)) {
        rotationScale = Cesium.Matrix4.getRotation(transform, scratchMatrix);
    } else {
        rotationScale = Cesium.Matrix4.getMatrix3(transform, scratchMatrix);
    }

    halfAxes = Cesium.Matrix3.multiply(rotationScale, halfAxes, halfAxes);

    if (Cesium.defined(result) && result instanceof Cesium.TileOrientedBoundingBox) {
        result.update(center, halfAxes);
        return result;
    }

    return new Cesium.TileOrientedBoundingBox(center, halfAxes);
}

function createRegion(region, transform, initialTransform, result) {
    if (!Cesium.Matrix4.equalsEpsilon(transform, initialTransform, Cesium.Math.EPSILON8)) {
        return createBoxFromTransformedRegion(region, transform, initialTransform, result);
    }

    if (Cesium.defined(result)) {
        // Don't update regions when the transform changes
        return result;
    }

    const rectangleRegion = Cesium.Rectangle.unpack(region, 0, scratchRectangle);

    return new Cesium.TileBoundingRegion({
        rectangle: rectangleRegion,
        minimumHeight: region[4],
        maximumHeight: region[5]
    });
}

function createSphere(sphere, transform, result) {
    let center = Cesium.Cartesian3.fromElements(sphere[0], sphere[1], sphere[2], scratchCenter);
    let radius = sphere[3];

    // Find the transformed center and radius
    center = Cesium.Matrix4.multiplyByPoint(transform, center, center);
    const scale = Cesium.Matrix4.getScale(transform, scratchScale);
    const uniformScale = Cesium.Cartesian3.maximumComponent(scale);
    radius *= uniformScale;

    if (Cesium.defined(result)) {
        result.update(center, radius);
        return result;
    }
    return new Cesium.TileBoundingSphere(center, radius);
}

function applyDebugSettings(tileParam, tileset, frameState) {
    const tile = tileParam;
    if (!frameState.passes.render) {
        return;
    }

    const hasContentBoundingVolume =
        Cesium.defined(tile._header.content) && Cesium.defined(tile._header.content.boundingVolume);
    const empty = tile.hasEmptyContent || tile.hasTilesetContent;

    const showVolume =
        tileset.debugShowBoundingVolume || (tileset.debugShowContentBoundingVolume && !hasContentBoundingVolume);
    if (showVolume) {
        let color;
        if (!tile._finalResolution) {
            color = Cesium.Color.YELLOW;
        } else if (empty) {
            color = Cesium.Color.DARKGRAY;
        } else {
            color = Cesium.Color.WHITE;
        }
        if (!Cesium.defined(tile._debugBoundingVolume)) {
            tile._debugBoundingVolume = tile._boundingVolume.createDebugVolume(color);
        }
        tile._debugBoundingVolume.update(frameState);
        const attributes = tile._debugBoundingVolume.getGeometryInstanceAttributes('outline');
        attributes.color = Cesium.ColorGeometryInstanceAttribute.toValue(color, attributes.color);
    } else if (!showVolume && Cesium.defined(tile._debugBoundingVolume)) {
        tile._debugBoundingVolume = tile._debugBoundingVolume.destroy();
    }

    if (tileset.debugShowContentBoundingVolume && hasContentBoundingVolume) {
        if (!Cesium.defined(tile._debugContentBoundingVolume)) {
            tile._debugContentBoundingVolume = tile._contentBoundingVolume.createDebugVolume(Cesium.Color.BLUE);
        }
        tile._debugContentBoundingVolume.update(frameState);
    } else if (!tileset.debugShowContentBoundingVolume && Cesium.defined(tile._debugContentBoundingVolume)) {
        tile._debugContentBoundingVolume = tile._debugContentBoundingVolume.destroy();
    }

    if (tileset.debugShowViewerRequestVolume && Cesium.defined(tile._viewerRequestVolume)) {
        if (!Cesium.defined(tile._debugViewerRequestVolume)) {
            tile._debugViewerRequestVolume = tile._viewerRequestVolume.createDebugVolume(Cesium.Color.YELLOW);
        }
        tile._debugViewerRequestVolume.update(frameState);
    } else if (!tileset.debugShowViewerRequestVolume && Cesium.defined(tile._debugViewerRequestVolume)) {
        tile._debugViewerRequestVolume = tile._debugViewerRequestVolume.destroy();
    }

    const debugColorizeTilesOn =
        (tileset.debugColorizeTiles && !tile._debugColorizeTiles) || Cesium.defined(tileset._heatmap.tilePropertyName);
    const debugColorizeTilesOff = !tileset.debugColorizeTiles && tile._debugColorizeTiles;

    if (debugColorizeTilesOn) {
        tileset._heatmap.colorize(tile, frameState); // Skipped if tileset._heatmap.tilePropertyName is undefined
        tile._debugColorizeTiles = true;
        tile.color = tile._debugColor;
    } else if (debugColorizeTilesOff) {
        tile._debugColorizeTiles = false;
        tile.color = Cesium.Color.WHITE;
    }

    if (tile._colorDirty) {
        tile._colorDirty = false;
        tile._content.applyDebugSettings(true, tile._color);
    }

    if (debugColorizeTilesOff) {
        tileset.makeStyleDirty(); // Re-apply style now that colorize is switched off
    }
}

function updateContent(tileParam, tileset, frameState) {
    const tile = tileParam;
    const content = tile._content;
    const expiredContent = tile._expiredContent;

    if (Cesium.defined(expiredContent)) {
        if (!tile.contentReady) {
            // Render the expired content while the content loads
            expiredContent.update(tileset, frameState);
            return;
        }

        // New content is ready, destroy expired content
        tile._expiredContent.destroy();
        tile._expiredContent = undefined;
    }

    content.update(tileset, frameState);
}

function updateClippingPlanes(tileParam, tileset) {
    // Compute and compare ClippingPlanes state:
    // - enabled-ness - are clipping planes enabled? is this tile clipped?
    // - clipping plane count
    // - clipping function (union v. intersection)
    const tile = tileParam;
    const { clippingPlanes } = tileset;
    let currentClippingPlanesState = 0;
    if (Cesium.defined(clippingPlanes) && tile._isClipped && clippingPlanes.enabled) {
        currentClippingPlanesState = clippingPlanes.clippingPlanesState;
    }
    // If clippingPlaneState for tile changed, mark clippingPlanesDirty so content can update
    if (currentClippingPlanesState !== tile._clippingPlanesState) {
        tile._clippingPlanesState = currentClippingPlanesState;
        tile.clippingPlanesDirty = true;
    }
}

const scratchCommandList = [];

function isolateDigits(normalizedValue, numberOfDigits, leftShift) {
    // const scaled = normalizedValue * Math.pow(10, numberOfDigits);
    const scaled = normalizedValue * 10 ** numberOfDigits;
    const integer = parseInt(scaled, 10);
    return integer * 10 ** leftShift;
}

function priorityNormalizeAndClamp(value, minimum, maximum) {
    return Math.max(Cesium.Math.normalize(value, minimum, maximum) - Cesium.Math.EPSILON7, 0.0); // Subtract epsilon since we only want decimal digits present in the output.
}

export default class MapGISM3D {
    // This can be overridden for testing purposes
    static _deprecationWarning() {
        return deprecationWarning;
    }

    constructor(tileset, baseResourceParam, header, parent) {
        let baseResource = baseResourceParam;
        this._tileset = tileset;
        this._header = header;
        const contentHeader = header.content;

        /**
         * The local transform of this tile.
         * @type {Matrix4}
         */
        this.transform = Cesium.defined(header.transform)
            ? Cesium.Matrix4.unpack(header.transform)
            : Cesium.Matrix4.clone(Cesium.Matrix4.IDENTITY);

        const parentTransform = Cesium.defined(parent) ? parent.computedTransform : tileset.modelMatrix;
        const computedTransform = Cesium.Matrix4.multiply(parentTransform, this.transform, new Cesium.Matrix4());

        const parentInitialTransform = Cesium.defined(parent) ? parent._initialTransform : Cesium.Matrix4.IDENTITY;
        this._initialTransform = Cesium.Matrix4.multiply(parentInitialTransform, this.transform, new Cesium.Matrix4());

        /**
         * The final computed transform of this tile.
         * @type {Matrix4}
         * @readonly
         */
        this.computedTransform = computedTransform;

        this._boundingVolume = this.createBoundingVolume(header.boundingVolume, computedTransform);
        this._boundingVolume2D = undefined;

        let contentBoundingVolume;

        if (Cesium.defined(contentHeader) && Cesium.defined(contentHeader.boundingVolume)) {
            // Non-leaf tiles may have a content bounding-volume, which is a tight-fit bounding volume
            // around only the features in the tile.  This box is useful for culling for rendering,
            // but not for culling for traversing the tree since it does not guarantee spatial coherence, i.e.,
            // since it only bounds features in the tile, not the entire tile, children may be
            // outside of this box.
            contentBoundingVolume = this.createBoundingVolume(contentHeader.boundingVolume, computedTransform);
        }
        this._contentBoundingVolume = contentBoundingVolume;
        this._contentBoundingVolume2D = undefined;

        let viewerRequestVolume;
        if (Cesium.defined(header.viewerRequestVolume)) {
            viewerRequestVolume = this.createBoundingVolume(header.viewerRequestVolume, computedTransform);
        }
        this._viewerRequestVolume = viewerRequestVolume;

        /**
         * The error, in meters, introduced if this tile is rendered and its children are not.
         * This is used to compute screen space error, i.e., the error measured in pixels.
         *
         * @type {Number}
         * @readonly
         */
        this.geometricError = header.geometricError;

        if (!Cesium.defined(this.geometricError)) {
            this.geometricError = Cesium.defined(parent) ? parent.geometricError : tileset._geometricError;
            MapGISM3D._deprecationWarning(
                'geometricErrorUndefined',
                "Required property geometricError is undefined for this tile. Using parent's geometric error instead."
            );
        }

        let refine;
        if (Cesium.defined(header.refine)) {
            if (header.refine === 'replace' || header.refine === 'add') {
                MapGISM3D._deprecationWarning(
                    'lowercase-refine',
                    `This tile uses a lowercase refine "${
                        header.refine
                    }". Instead use "${header.refine.toUpperCase()}".`
                );
            }
            refine =
                header.refine.toUpperCase() === 'REPLACE'
                    ? Cesium.Cesium3DTileRefine.REPLACE
                    : Cesium.Cesium3DTileRefine.ADD;
        } else if (Cesium.defined(parent)) {
            // Inherit from parent tile if omitted.
            refine = parent.refine;
        } else {
            refine = Cesium.Cesium3DTileRefine.REPLACE;
        }

        /**
         * Specifies the type of refinment that is used when traversing this tile for rendering.
         *
         * @type {Cesium3DTileRefine}
         * @readonly
         * @private
         */
        this.refine = refine;

        /**
         * Gets the tile's children.
         *
         * @type {MapGISM3D[]}
         * @readonly
         */
        this.children = [];

        // hys 添加孩子列表 避免重复添加
        this.childrenNameList = [];

        /**
         * This tile's parent or <code>undefined</code> if this tile is the root.
         * <p>
         * When a tile's content points to an external tileset JSON file, the external tileset's
         * root tile's parent is not <code>undefined</code>; instead, the parent references
         * the tile (with its content pointing to an external tileset JSON file) as if the two tilesets were merged.
         * </p>
         *
         * @type {MapGISM3D}
         * @readonly
         */
        this.parent = parent;

        let content;
        let hasEmptyContent;
        let contentState;
        let contentResource;
        let serverKey;

        baseResource = Cesium.Resource.createIfNeeded(baseResource);

        if (Cesium.defined(contentHeader)) {
            let contentHeaderUri = contentHeader.uri;
            if (Cesium.defined(contentHeader.url)) {
                MapGISM3D._deprecationWarning(
                    'contentUrl',
                    'This tileset JSON uses the "content.url" property which has been deprecated. Use "content.uri" instead.'
                );
                contentHeaderUri = contentHeader.url;
            }

            hasEmptyContent = false;
            contentState = Cesium.Cesium3DTileContentState.UNLOADED;
            if (!this._tileset._isIGServer) {
                contentResource = baseResource.getDerivedResource({
                    url: contentHeaderUri
                });
            } else {
                let dataName = '';
                contentResource = baseResource.clone();
                // if(contentHeaderUrl.charAt(0) !== '/'){
                if (contentHeaderUri.lastIndexOf('/') < 0) {
                    const pos1 = contentResource.url.lastIndexOf('dataName=');
                    const pos2 = contentResource.url.lastIndexOf('&webGL=');
                    dataName = contentResource.url.substring(pos1 + 9, pos2);
                    dataName = decodeURIComponent(dataName);

                    const tempDataName = dataName.substring(0, dataName.lastIndexOf('/') + 1);
                    dataName = tempDataName;
                }

                if (contentHeaderUri.indexOf('..') === 0) {
                    // fgy 兼容优化后m3d数据请求地址
                    contentHeaderUri = contentHeaderUri.replace('..', 'data');
                }

                contentResource.url = contentResource.url.replace('datatype=10', 'datatype=11');
                // 韩彦生 添加对m3d 地理服务图层的判断支持
                if (this._tileset._isM3dDataServer > -1) {
                    // contentHeaderUri = contentHeaderUri.replace('/','\\');
                    // contentHeaderUri = contentHeaderUri.replace('/','\\');
                    // contentResource.url = contentResource.url +'&dataName='+ encodeURIComponent(dataName +contentHeaderUri)+ '&compress=false';
                    // contentResource.url = contentResource.url.replace('{dataName}',encodeURIComponent(dataName +contentHeaderUri)) ;///+ '&compress=false';
                    contentResource.url = `${
                        contentResource.url.substring(0, contentResource.url.lastIndexOf('&dataName=') + 10) +
                        encodeURIComponent(dataName + contentHeaderUri)
                    }&webGL=true`; /// + '&compress=false';
                } else {
                    // contentResource._url = contentResource.url;//这里是为方便子节点使用
                    // hys 对地址中有+ 等特殊字符的做编码处理 不然会把这些特殊字符漏掉 encodeURIComponent
                    contentResource.url = `${contentResource.url.substring(
                        0,
                        contentResource.url.lastIndexOf('datatype=') + 11
                    )}&dataName=${encodeURIComponent(dataName + contentHeaderUri)}&webGL=true&compress=false`;
                    // contentResource.url = contentResource.url.replace('{dataName}', contentHeaderUrl);
                }
            }
            serverKey = Cesium.RequestScheduler.getServerKey(contentResource.getUrlComponent());
        } else {
            content = new Cesium.Empty3DTileContent(tileset, this);
            hasEmptyContent = true;
            contentState = Cesium.Cesium3DTileContentState.READY;
        }

        this._content = content;
        this._contentResource = contentResource;
        this._contentState = contentState;
        this._contentReadyToProcessPromise = undefined;
        this._contentReadyPromise = undefined;
        this._expiredContent = undefined;

        this._serverKey = serverKey;

        /**
         * When <code>true</code>, the tile has no content.
         *
         * @type {Boolean}
         * @readonly
         *
         * @private
         */
        this.hasEmptyContent = hasEmptyContent;

        /**
         * When <code>true</code>, the tile's content points to an external tileset.
         * <p>
         * This is <code>false</code> until the tile's content is loaded.
         * </p>
         *
         * @type {Boolean}
         * @readonly
         *
         * @private
         */
        this.hasTilesetContent = false;

        /**
         * The node in the tileset's LRU cache, used to determine when to unload a tile's content.
         *
         * See {@link Cesium3DTilesetCache}
         *
         * @type {DoublyLinkedListNode}
         * @readonly
         *
         * @private
         */
        this.cacheNode = undefined;

        const { expire } = header;
        let expireDuration;
        let expireDate;
        if (Cesium.defined(expire)) {
            expireDuration = expire.duration;
            if (Cesium.defined(expire.date)) {
                expireDate = Cesium.JulianDate.fromIso8601(expire.date);
            }
        }

        /**
         * The time in seconds after the tile's content is ready when the content expires and new content is requested.
         *
         * @type {Number}
         */
        this.expireDuration = expireDuration;

        /**
         * The date when the content expires and new content is requested.
         *
         * @type {JulianDate}
         */
        this.expireDate = expireDate;

        /**
         * The time when a style was last applied to this tile.
         *
         * @type {Number}
         *
         * @private
         */
        this.lastStyleTime = 0;

        /**
         * Marks whether the tile's children bounds are fully contained within the tile's bounds
         *
         * @type {Cesium3DTileOptimizationHint}
         *
         * @private
         */
        this._optimChildrenWithinParent = Cesium.Cesium3DTileOptimizationHint.NOT_COMPUTED;

        /**
         * Tracks if the tile's relationship with a ClippingPlaneCollection has changed with regards
         * to the ClippingPlaneCollection's state.
         *
         * @type {Boolean}
         *
         * @private
         */
        this.clippingPlanesDirty = false;

        /**
         * Tracks if the tile's request should be deferred until all non-deferred
         * tiles load.
         *
         * @type {Boolean}
         *
         * @private
         */
        this.priorityDeferred = false;

        // Members that are updated every frame for tree traversal and rendering optimizations:
        this._distanceToCamera = 0.0;
        this._centerZDepth = 0.0;
        this._screenSpaceError = 0.0;
        this._screenSpaceErrorProgressiveResolution = 0.0; // The screen space error at a given screen height of tileset.progressiveResolutionHeightFraction * screenHeight
        this._visibilityPlaneMask = 0;
        this._visible = false;
        this._inRequestVolume = false;

        this._finalResolution = true;
        this._depth = 0;
        this._stackLength = 0;
        this._selectionDepth = 0;

        this._updatedVisibilityFrame = 0;
        this._touchedFrame = 0;
        this._visitedFrame = 0;
        this._selectedFrame = 0;
        this._requestedFrame = 0;
        this._ancestorWithContent = undefined;
        this._ancestorWithContentAvailable = undefined;
        this._refines = false;
        this._shouldSelect = false;
        this._isClipped = true;
        this._clippingPlanesState = 0; // encapsulates (_isClipped, clippingPlanes.enabled) and number/function
        this._debugBoundingVolume = undefined;
        this._debugContentBoundingVolume = undefined;
        this._debugViewerRequestVolume = undefined;
        this._debugColor = Cesium.Color.fromRandom({
            alpha: 1.0
        });
        this._debugColorizeTiles = false;

        this._priority = 0.0; // The priority used for request sorting
        this._priorityHolder = this; // Reference to the ancestor up the tree that holds the _foveatedFactor and _distanceToCamera for all tiles in the refinement chain.
        this._priorityProgressiveResolution = false;
        this._priorityProgressiveResolutionScreenSpaceErrorLeaf = false;
        this._priorityReverseScreenSpaceError = 0.0;
        this._foveatedFactor = 0.0;
        this._wasMinPriorityChild = false; // Needed for knowing when to continue a refinement chain. Gets reset in updateTile in traversal and gets set in updateAndPushChildren in traversal.

        this._loadTimestamp = new Cesium.JulianDate();

        this._commandsLength = 0;

        this._color = undefined;
        this._colorDirty = false;
    }

    /**
     * The tileset containing this tile.
     *
     * @memberof MapGISM3D.prototype
     *
     * @type {Cesium3DTileset}
     * @readonly
     */
    get tileset() {
        return this._tileset;
    }

    /**
     * The tile's content.  This represents the actual tile's payload,
     * not the content's metadata in tileset.json.
     *
     * @memberof MapGISM3D.prototype
     *
     * @type {Cesium3DTileContent}
     * @readonly
     */
    get content() {
        return this._content;
    }

    /**
     * Get the tile's bounding volume.
     *
     * @memberof MapGISM3D.prototype
     *
     * @type {TileBoundingVolume}
     * @readonly
     * @private
     */
    get boundingVolume() {
        return this._boundingVolume;
    }

    /**
     * Get the bounding volume of the tile's contents.  This defaults to the
     * tile's bounding volume when the content's bounding volume is
     * <code>undefined</code>.
     *
     * @memberof MapGISM3D.prototype
     *
     * @type {TileBoundingVolume}
     * @readonly
     * @private
     */
    get contentBoundingVolume() {
        return Cesium.defaultValue(this._contentBoundingVolume, this._boundingVolume);
    }

    /**
     * Get the bounding sphere derived from the tile's bounding volume.
     *
     * @memberof MapGISM3D.prototype
     *
     * @type {BoundingSphere}
     * @readonly
     */
    get boundingSphere() {
        return this._boundingVolume.boundingSphere;
    }

    /**
     * Returns the <code>extras</code> property in the tileset JSON for this tile, which contains application specific metadata.
     * Returns <code>undefined</code> if <code>extras</code> does not exist.
     *
     * @memberof MapGISM3D.prototype
     *
     * @type {*}
     * @readonly
     * @see {@link https://github.com/AnalyticalGraphicsInc/3d-tiles/tree/master/specification#specifying-extensions-and-application-specific-extras|Extras in the 3D Tiles specification.}
     */
    get extras() {
        return this._header.extras;
    }

    /**
     * Gets or sets the tile's highlight color.
     *
     * @memberof MapGISM3D.prototype
     *
     * @type {Color}
     *
     * @default {@link Color.WHITE}
     *
     * @private
     */
    get color() {
        if (!Cesium.defined(this._color)) {
            this._color = new Cesium.Color();
        }
        return Cesium.Color.clone(this._color);
    }

    set color(value) {
        this._color = Cesium.Color.clone(value, this._color);
        this._colorDirty = true;
    }

    /**
     * Determines if the tile has available content to render.  <code>true</code> if the tile's
     * content is ready or if it has expired content that renders while new content loads; otherwise,
     * <code>false</code>.
     *
     * @memberof MapGISM3D.prototype
     *
     * @type {Boolean}
     * @readonly
     *
     * @private
     */
    get contentAvailable() {
        return (
            (this.contentReady && !this.hasEmptyContent && !this.hasTilesetContent) ||
            (Cesium.defined(this._expiredContent) && !this.contentFailed)
        );
    }

    /**
     * Determines if the tile's content is ready. This is automatically <code>true</code> for
     * tile's with empty content.
     *
     * @memberof MapGISM3D.prototype
     *
     * @type {Boolean}
     * @readonly
     *
     * @private
     */
    get contentReady() {
        return this._contentState === Cesium.Cesium3DTileContentState.READY;
    }

    /**
     * Determines if the tile's content has not be requested. <code>true</code> if tile's
     * content has not be requested; otherwise, <code>false</code>.
     *
     * @memberof MapGISM3D.prototype
     *
     * @type {Boolean}
     * @readonly
     *
     * @private
     */
    get contentUnloaded() {
        return this._contentState === Cesium.Cesium3DTileContentState.UNLOADED;
    }

    /**
     * Determines if the tile's content is expired. <code>true</code> if tile's
     * content is expired; otherwise, <code>false</code>.
     *
     * @memberof MapGISM3D.prototype
     *
     * @type {Boolean}
     * @readonly
     *
     * @private
     */
    get contentExpired() {
        return this._contentState === Cesium.Cesium3DTileContentState.EXPIRED;
    }

    /**
     * Determines if the tile's content failed to load.  <code>true</code> if the tile's
     * content failed to load; otherwise, <code>false</code>.
     *
     * @memberof MapGISM3D.prototype
     *
     * @type {Boolean}
     * @readonly
     *
     * @private
     */
    get contentFailed() {
        return this._contentState === Cesium.Cesium3DTileContentState.FAILED;
    }

    /**
     * Gets the promise that will be resolved when the tile's content is ready to process.
     * This happens after the content is downloaded but before the content is ready
     * to render.
     * <p>
     * The promise remains <code>undefined</code> until the tile's content is requested.
     * </p>
     *
     * @type {Promise.<Cesium3DTileContent>}
     * @readonly
     *
     * @private
     */
    get contentReadyToProcessPromise() {
        if (Cesium.defined(this._contentReadyToProcessPromise)) {
            return this._contentReadyToProcessPromise.promise;
        }

        return undefined;
    }

    /**
     * Gets the promise that will be resolved when the tile's content is ready to render.
     * <p>
     * The promise remains <code>undefined</code> until the tile's content is requested.
     * </p>
     *
     * @type {Promise.<Cesium3DTileContent>}
     * @readonly
     *
     * @private
     */
    get contentReadyPromise() {
        if (Cesium.defined(this._contentReadyPromise)) {
            return this._contentReadyPromise.promise;
        }

        return undefined;
    }

    /**
     * Returns the number of draw commands used by this tile.
     *
     * @readonly
     *
     * @private
     */
    get commandsLength() {
        return this._commandsLength;
    }

    /**
     * Get the tile's screen space error.
     *
     * @private
     */
    getScreenSpaceError(frameStateParam, useParentGeometricErrorParam, progressiveResolutionHeightFractionParam) {
        const frameState = frameStateParam;
        const useParentGeometricError = useParentGeometricErrorParam;
        const progressiveResolutionHeightFraction = progressiveResolutionHeightFractionParam;

        const tileset = this._tileset;
        const heightFraction = Cesium.defaultValue(progressiveResolutionHeightFraction, 1.0);
        const parentGeometricError = Cesium.defined(this.parent) ? this.parent.geometricError : tileset._geometricError;
        const geometricError = useParentGeometricError ? parentGeometricError : this.geometricError;
        if (geometricError === 0.0) {
            // Leaf tiles do not have any error so save the computation
            return 0.0;
        }
        const { camera } = frameState;
        let { frustum } = camera;
        const { context } = frameState;
        const width = context.drawingBufferWidth;
        const height = context.drawingBufferHeight * heightFraction;
        let error;
        if (frameState.mode === Cesium.SceneMode.SCENE2D || frustum instanceof Cesium.OrthographicFrustum) {
            if (Cesium.defined(frustum._offCenterFrustum)) {
                frustum = frustum._offCenterFrustum;
            }
            const pixelSize =
                Math.max(frustum.top - frustum.bottom, frustum.right - frustum.left) / Math.max(width, height);
            error = geometricError / pixelSize;
        } else {
            // Avoid divide by zero when viewer is inside the tile
            const distance = Math.max(this._distanceToCamera, Cesium.Math.EPSILON7);
            const { sseDenominator } = camera.frustum;
            error = (geometricError * height) / (distance * sseDenominator);
            if (tileset.dynamicScreenSpaceError) {
                const density = tileset._dynamicScreenSpaceErrorComputedDensity;
                const factor = tileset.dynamicScreenSpaceErrorFactor;
                const dynamicError = Cesium.Math.fog(distance, density) * factor;
                error -= dynamicError;
            }
        }
        return error;
    }

    /**
     * Update the tile's visibility.
     *
     * @private
     */
    updateVisibility(frameStateParam) {
        const frameState = frameStateParam;
        const { parent } = this;
        const tileset = this._tileset;
        const parentTransform = Cesium.defined(parent) ? parent.computedTransform : tileset.modelMatrix;
        const parentVisibilityPlaneMask = Cesium.defined(parent)
            ? parent._visibilityPlaneMask
            : Cesium.CullingVolume.MASK_INDETERMINATE;
        this.updateTransform(parentTransform);
        this._distanceToCamera = this.distanceToTile(frameState);
        this._centerZDepth = this.distanceToTileCenter(frameState);
        this._screenSpaceError = this.getScreenSpaceError(frameState, false);
        this._screenSpaceErrorProgressiveResolution = this.getScreenSpaceError(
            frameState,
            false,
            tileset.progressiveResolutionHeightFraction
        );
        this._visibilityPlaneMask = this.visibility(frameState, parentVisibilityPlaneMask); // Use parent's plane mask to speed up visibility test
        this._visible = this._visibilityPlaneMask !== Cesium.CullingVolume.MASK_OUTSIDE;
        this._inRequestVolume = this.insideViewerRequestVolume(frameState);
        this._priorityReverseScreenSpaceError = getPriorityReverseScreenSpaceError(tileset, this);
        this._priorityProgressiveResolution = isPriorityProgressiveResolution(tileset, this);
        this.priorityDeferred = isPriorityDeferred(this, frameState);
    }

    /**
     * Update whether the tile has expired.
     *
     * @private
     */
    updateExpiration() {
        if (Cesium.defined(this.expireDate) && this.contentReady && !this.hasEmptyContent) {
            const now = Cesium.JulianDate.now(scratchJulianDate);
            if (Cesium.JulianDate.lessThan(this.expireDate, now)) {
                this._contentState = Cesium.Cesium3DTileContentState.EXPIRED;
                this._expiredContent = this._content;
            }
        }
    }

    /**
     * Requests the tile's content.
     * <p>
     * The request may not be made if the Cesium Request Scheduler can't prioritize it.
     * </p>
     *
     * @private
     */
    requestContent() {
        const that = this;
        const tileset = this._tileset;

        if (this.hasEmptyContent) {
            return false;
        }

        const resource = this._contentResource.clone();
        const expired = this.contentExpired;
        if (expired) {
            // Append a query parameter of the tile expiration date to prevent caching
            resource.setQueryParameters({
                expired: this.expireDate.toString()
            });
        }

        const request = new Cesium.Request({
            throttle: true,
            throttleByServer: true,
            type: Cesium.RequestType.TILES3D,
            priorityFunction: createPriorityFunction(this),
            serverKey: this._serverKey
        });

        this._request = request;
        resource.request = request;

        const promise = resource.fetchArrayBuffer();

        if (!Cesium.defined(promise)) {
            return false;
        }

        const contentState = this._contentState;
        this._contentState = Cesium.Cesium3DTileContentState.LOADING;
        this._contentReadyToProcessPromise = Cesium.when.defer();
        this._contentReadyPromise = Cesium.when.defer();

        if (expired) {
            this.expireDate = undefined;
        }

        const contentFailedFunction = getContentFailedFunction(this);

        promise
            .then((arrayBufferResult) => {
                if (that.isDestroyed()) {
                    // Tile is unloaded before the content finishes loading
                    contentFailedFunction();
                    return;
                }
                let arrayBuffer = arrayBufferResult;
                let uint8Array = new Uint8Array(arrayBuffer);
                let magic = Cesium.getMagic(uint8Array);

                if (magic === 'zipx') {
                    uint8Array = uint8Array.slice(3);

                    const inflator = new Zlib.Inflate(uint8Array);
                    const inflated = inflator.decompress();

                    arrayBuffer = inflated.buffer;
                }

                magic = 'm3d';
                if (!Cesium.defined(Cesium.Cesium3DTileContentFactory.m3d)) {
                    Cesium.Cesium3DTileContentFactory.m3d = (
                        tilesetParam,
                        tileParam,
                        resourceParam,
                        arrayBufferParam,
                        byteOffsetParam
                    ) => {
                        return new MapGISM3DDataContent(
                            tilesetParam,
                            tileParam,
                            resourceParam,
                            arrayBufferParam,
                            byteOffsetParam
                        );
                    };
                }
                const contentFactory = Cesium.Cesium3DTileContentFactory[magic];
                // const contentFactory = M3DTileContentFactory[magic];
                let content;

                // Vector and Geometry tile rendering do not support the skip LOD optimization.
                tileset._disableSkipLevelOfDetail =
                    tileset._disableSkipLevelOfDetail || magic === 'vctr' || magic === 'geom';

                if (Cesium.defined(contentFactory)) {
                    content = contentFactory(tileset, that, that._contentResource, arrayBuffer, 0);
                } else {
                    // 此处单独创建 M3D 数据对象，不从 Factory 中获取
                    content = new MapGISM3DDataContent(tileset, that, that._contentResource, arrayBuffer, 0);

                    that.hasTilesetContent = true;
                }
                that._content = content;
                that._contentState = Cesium.Cesium3DTileContentState.PROCESSING;
                that._contentReadyToProcessPromise.resolve(content);
                // 韩彦生
                if (content.readyPromise === undefined) {
                    return;
                }

                // eslint-disable-next-line consistent-return
                return content.readyPromise.then((contentParam) => {
                    if (that.isDestroyed()) {
                        // Tile is unloaded before the content finishes processing
                        contentFailedFunction();
                        // return;
                    } else {
                        updateExpireDate(that);

                        // Refresh style for expired content
                        that._selectedFrame = 0;
                        that.lastStyleTime = 0.0;

                        Cesium.JulianDate.now(that._loadTimestamp);
                        that._contentState = Cesium.Cesium3DTileContentState.READY;
                        that._contentReadyPromise.resolve(contentParam);
                    }
                });
            })
            .otherwise((error) => {
                // debugger
                if (request.state === Cesium.RequestState.CANCELLED) {
                    // Cancelled due to low priority - try again later.
                    that._contentState = contentState;
                    tileset.statistics.numberOfPendingRequests -= 1;
                    tileset.statistics.numberOfAttemptedRequests += 1;
                    return;
                }
                contentFailedFunction(error);
            });

        return true;
    }

    unloadContent() {
        if (this.hasEmptyContent || this.hasTilesetContent) {
            return;
        }

        this._content = this._content && this._content.destroy();
        this._contentState = Cesium.Cesium3DTileContentState.UNLOADED;
        this._contentReadyToProcessPromise = undefined;
        this._contentReadyPromise = undefined;

        this.lastStyleTime = 0.0;
        this.clippingPlanesDirty = this._clippingPlanesState === 0;
        this._clippingPlanesState = 0;

        this._debugColorizeTiles = false;

        this._debugBoundingVolume = this._debugBoundingVolume && this._debugBoundingVolume.destroy();
        this._debugContentBoundingVolume =
            this._debugContentBoundingVolume && this._debugContentBoundingVolume.destroy();
        this._debugViewerRequestVolume = this._debugViewerRequestVolume && this._debugViewerRequestVolume.destroy();
    }

    /**
     * Determines whether the tile's bounding volume intersects the culling volume.
     *
     * @param {FrameState} frameState The frame state.
     * @param {Number} parentVisibilityPlaneMask The parent's plane mask to speed up the visibility check.
     * @returns {Number} A plane mask as described above in {@link CullingVolume#computeVisibilityWithPlaneMask}.
     *
     * @private
     */
    visibility(frameState, parentVisibilityPlaneMask) {
        const { cullingVolume } = frameState;
        const boundingVolume = getBoundingVolume(this, frameState);

        const tileset = this._tileset;
        const { clippingPlanes } = tileset;
        if (Cesium.defined(clippingPlanes) && clippingPlanes.enabled) {
            const intersection = clippingPlanes.computeIntersectionWithBoundingVolume(
                boundingVolume,
                tileset.clippingPlanesOriginMatrix
            );
            this._isClipped = intersection !== Cesium.Intersect.INSIDE;
            if (intersection === Cesium.Intersect.OUTSIDE) {
                return Cesium.CullingVolume.MASK_OUTSIDE;
            }
        }

        return cullingVolume.computeVisibilityWithPlaneMask(boundingVolume, parentVisibilityPlaneMask);
    }

    /**
     * Assuming the tile's bounding volume intersects the culling volume, determines
     * whether the tile's content's bounding volume intersects the culling volume.
     *
     * @param {FrameState} frameState The frame state.
     * @returns {Intersect} The result of the intersection: the tile's content is completely outside, completely inside, or intersecting the culling volume.
     *
     * @private
     */
    contentVisibility(frameState) {
        // Assumes the tile's bounding volume intersects the culling volume already, so
        // just return Intersect.INSIDE if there is no content bounding volume.
        if (!Cesium.defined(this._contentBoundingVolume)) {
            return Cesium.Intersect.INSIDE;
        }

        if (this._visibilityPlaneMask === Cesium.CullingVolume.MASK_INSIDE) {
            // The tile's bounding volume is completely inside the culling volume so
            // the content bounding volume must also be inside.
            return Cesium.Intersect.INSIDE;
        }

        // PERFORMANCE_IDEA: is it possible to burn less CPU on this test since we know the
        // tile's (not the content's) bounding volume intersects the culling volume?
        const { cullingVolume } = frameState;
        const boundingVolume = getContentBoundingVolume(this, frameState);

        const tileset = this._tileset;
        const { clippingPlanes } = tileset;
        if (Cesium.defined(clippingPlanes) && clippingPlanes.enabled) {
            const intersection = clippingPlanes.computeIntersectionWithBoundingVolume(
                boundingVolume,
                tileset.clippingPlanesOriginMatrix
            );
            this._isClipped = intersection !== Cesium.Intersect.INSIDE;
            if (intersection === Cesium.Intersect.OUTSIDE) {
                return Cesium.Intersect.OUTSIDE;
            }
        }

        return cullingVolume.computeVisibility(boundingVolume);
    }

    /**
     * Computes the (potentially approximate) distance from the closest point of the tile's bounding volume to the camera.
     *
     * @param {FrameState} frameState The frame state.
     * @returns {Number} The distance, in meters, or zero if the camera is inside the bounding volume.
     *
     * @private
     */
    distanceToTile(frameState) {
        const boundingVolume = getBoundingVolume(this, frameState);
        return boundingVolume.distanceToCamera(frameState);
    }

    /**
     * Computes the distance from the center of the tile's bounding volume to the camera.
     *
     * @param {FrameState} frameState The frame state.
     * @returns {Number} The distance, in meters, or zero if the camera is inside the bounding volume.
     *
     * @private
     */
    distanceToTileCenter(frameState) {
        const tileBoundingVolume = getBoundingVolume(this, frameState);
        const { boundingVolume } = tileBoundingVolume; // Gets the underlying OrientedBoundingBox or BoundingSphere
        const toCenter = Cesium.Cartesian3.subtract(
            boundingVolume.center,
            frameState.camera.positionWC,
            scratchToTileCenter
        );
        return Cesium.Cartesian3.dot(frameState.camera.directionWC, toCenter);
    }

    /**
     * Checks if the camera is inside the viewer request volume.
     *
     * @param {FrameState} frameState The frame state.
     * @returns {Boolean} Whether the camera is inside the volume.
     *
     * @private
     */
    insideViewerRequestVolume(frameState) {
        const viewerRequestVolume = this._viewerRequestVolume;
        return !Cesium.defined(viewerRequestVolume) || viewerRequestVolume.distanceToCamera(frameState) === 0.0;
    }

    /**
     * Create a bounding volume from the tile's bounding volume header.
     *
     * @param {Object} boundingVolumeHeader The tile's bounding volume header.
     * @param {Matrix4} transform The transform to apply to the bounding volume.
     * @param {TileBoundingVolume} [result] The object onto which to store the result.
     *
     * @returns {TileBoundingVolume} The modified result parameter or a new TileBoundingVolume instance if none was provided.
     *
     * @private
     */
    createBoundingVolume(boundingVolumeHeader, transform, result) {
        if (!Cesium.defined(boundingVolumeHeader)) {
            throw new Cesium.RuntimeError('boundingVolume must be defined');
        }
        if (Cesium.defined(boundingVolumeHeader.box)) {
            return createBox(boundingVolumeHeader.box, transform, result);
        }
        if (Cesium.defined(boundingVolumeHeader.region)) {
            return createRegion(boundingVolumeHeader.region, transform, this._initialTransform, result);
        }
        if (Cesium.defined(boundingVolumeHeader.sphere)) {
            return createSphere(boundingVolumeHeader.sphere, transform, result);
        }
        throw new Cesium.RuntimeError('boundingVolume must contain a sphere, region, or box');
    }

    /**
     * Update the tile's transform. The transform is applied to the tile's bounding volumes.
     *
     * @private
     */
    updateTransform(parentTransformParam) {
        let parentTransform = parentTransformParam;
        parentTransform = Cesium.defaultValue(parentTransform, Cesium.Matrix4.IDENTITY);
        const computedTransform = Cesium.Matrix4.multiply(parentTransform, this.transform, scratchTransform);
        const transformChanged = !Cesium.Matrix4.equals(computedTransform, this.computedTransform);

        if (!transformChanged) {
            return;
        }

        Cesium.Matrix4.clone(computedTransform, this.computedTransform);

        // Update the bounding volumes
        const header = this._header;
        const { content } = this._header;
        this._boundingVolume = this.createBoundingVolume(
            header.boundingVolume,
            this.computedTransform,
            this._boundingVolume
        );
        if (Cesium.defined(this._contentBoundingVolume)) {
            this._contentBoundingVolume = this.createBoundingVolume(
                content.boundingVolume,
                this.computedTransform,
                this._contentBoundingVolume
            );
        }
        if (Cesium.defined(this._viewerRequestVolume)) {
            this._viewerRequestVolume = this.createBoundingVolume(
                header.viewerRequestVolume,
                this.computedTransform,
                this._viewerRequestVolume
            );
        }

        // Destroy the debug bounding volumes. They will be generated fresh.
        this._debugBoundingVolume = this._debugBoundingVolume && this._debugBoundingVolume.destroy();
        this._debugContentBoundingVolume =
            this._debugContentBoundingVolume && this._debugContentBoundingVolume.destroy();
        this._debugViewerRequestVolume = this._debugViewerRequestVolume && this._debugViewerRequestVolume.destroy();
    }

    /**
     * Get the draw commands needed to render this tile.
     *
     * @private
     */
    update(tileset, frameState) {
        const initCommandLength = frameState.commandList.length;
        updateClippingPlanes(this, tileset);
        applyDebugSettings(this, tileset, frameState);
        updateContent(this, tileset, frameState);
        this._commandsLength = frameState.commandList.length - initCommandLength;

        this.clippingPlanesDirty = false; // reset after content update
    }

    /**
     * Processes the tile's content, e.g., create WebGL resources, to move from the PROCESSING to READY state.
     *
     * @param {Cesium3DTileset} tileset The tileset containing this tile.
     * @param {FrameState} frameState The frame state.
     *
     * @private
     */
    process(tileset, frameStateParam) {
        const frameState = frameStateParam;
        const savedCommandList = frameState.commandList;
        frameState.commandList = scratchCommandList;

        this._content.update(tileset, frameState);

        scratchCommandList.length = 0;
        frameState.commandList = savedCommandList;
    }

    /**
     * Sets the priority of the tile based on distance and depth
     * @private
     */
    updatePriority() {
        const { tileset } = this;
        const { preferLeaves } = tileset;
        const minimumPriority = tileset._minimumPriority;
        const maximumPriority = tileset._maximumPriority;

        // Combine priority systems together by mapping them into a base 10 number where each priority controls a specific set of digits in the number.
        // For number priorities, map them to a 0.xxxxx number then left shift it up into a set number of digits before the decimal point. Chop of the fractional part then left shift again into the position it needs to go.
        // For blending number priorities, normalize them to 0-1 and interpolate to get a combined 0-1 number, then proceed as normal.
        // Booleans can just be 0 or 10^leftshift.
        // Think of digits as penalties since smaller numbers are higher priority. If a tile has some large quantity or has a flag raised it's (usually) penalized for it, expressed as a higher number for the digit.
        // Priority number format: preloadFlightDigits(1) | foveatedDeferDigits(1) | foveatedDigits(4) | preloadProgressiveResolutionDigits(1) | preferredSortingDigits(4) . depthDigits(the decimal digits)
        // Certain flags like preferLeaves will flip / turn off certain digits to get desired load order.

        // Setup leftShifts, digit counts, and scales (for booleans)
        const digitsForANumber = 4;
        const digitsForABoolean = 1;

        const preferredSortingLeftShift = 0;
        const preferredSortingDigitsCount = digitsForANumber;

        const foveatedLeftShift = preferredSortingLeftShift + preferredSortingDigitsCount;
        const foveatedDigitsCount = digitsForANumber;

        const preloadProgressiveResolutionLeftShift = foveatedLeftShift + foveatedDigitsCount;
        const preloadProgressiveResolutionDigitsCount = digitsForABoolean;
        // const preloadProgressiveResolutionScale = Math.pow(10, preloadProgressiveResolutionLeftShift);
        const preloadProgressiveResolutionScale = 10 ** preloadProgressiveResolutionLeftShift;

        const foveatedDeferLeftShift = preloadProgressiveResolutionLeftShift + preloadProgressiveResolutionDigitsCount;
        const foveatedDeferDigitsCount = digitsForABoolean;
        // const foveatedDeferScale = Math.pow(10, foveatedDeferLeftShift);
        const foveatedDeferScale = 10 ** foveatedDeferLeftShift;

        const preloadFlightLeftShift = foveatedDeferLeftShift + foveatedDeferDigitsCount;
        // const preloadFlightScale = Math.pow(10, preloadFlightLeftShift);
        const preloadFlightScale = 10 ** preloadFlightLeftShift;

        // Compute the digits for each priority
        let depthDigits = priorityNormalizeAndClamp(this._depth, minimumPriority.depth, maximumPriority.depth);
        depthDigits = preferLeaves ? 1.0 - depthDigits : depthDigits;

        // Map 0-1 then convert to digit. Include a distance sort when doing non-skipLOD and replacement refinement, helps things like non-skipLOD photogrammetry
        const useDistance = !tileset._skipLevelOfDetail && this.refine === Cesium.Cesium3DTileRefine.REPLACE;
        const normalizedPreferredSorting = useDistance
            ? priorityNormalizeAndClamp(
                  this._priorityHolder._distanceToCamera,
                  minimumPriority.distance,
                  maximumPriority.distance
              )
            : priorityNormalizeAndClamp(
                  this._priorityReverseScreenSpaceError,
                  minimumPriority.reverseScreenSpaceError,
                  maximumPriority.reverseScreenSpaceError
              );
        const preferredSortingDigits = isolateDigits(
            normalizedPreferredSorting,
            preferredSortingDigitsCount,
            preferredSortingLeftShift
        );

        const preloadProgressiveResolutionDigits = this._priorityProgressiveResolution
            ? 0
            : preloadProgressiveResolutionScale;

        const normalizedFoveatedFactor = priorityNormalizeAndClamp(
            this._priorityHolder._foveatedFactor,
            minimumPriority.foveatedFactor,
            maximumPriority.foveatedFactor
        );
        const foveatedDigits = isolateDigits(normalizedFoveatedFactor, foveatedDigitsCount, foveatedLeftShift);

        const foveatedDeferDigits = this.priorityDeferred ? foveatedDeferScale : 0;

        const preloadFlightDigits = tileset._pass === Cesium.Cesium3DTilePass.PRELOAD_FLIGHT ? 0 : preloadFlightScale;

        // Get the final base 10 number
        this._priority =
            depthDigits +
            preferredSortingDigits +
            preloadProgressiveResolutionDigits +
            foveatedDigits +
            foveatedDeferDigits +
            preloadFlightDigits;
    }

    /**
     * @private
     */
    // eslint-disable-next-line class-methods-use-this
    isDestroyed() {
        return false;
    }

    /**
     * @private
     */
    destroy() {
        // For the interval between new content being requested and downloaded, expiredContent === content, so don't destroy twice
        this._content = this._content && this._content.destroy();
        this._expiredContent =
            this._expiredContent && !this._expiredContent.isDestroyed() && this._expiredContent.destroy();
        this._debugBoundingVolume = this._debugBoundingVolume && this._debugBoundingVolume.destroy();
        this._debugContentBoundingVolume =
            this._debugContentBoundingVolume && this._debugContentBoundingVolume.destroy();
        this._debugViewerRequestVolume = this._debugViewerRequestVolume && this._debugViewerRequestVolume.destroy();
        return Cesium.destroyObject(this);
    }
}

CesiumZondy.M3D.MapGISM3D = MapGISM3D;
