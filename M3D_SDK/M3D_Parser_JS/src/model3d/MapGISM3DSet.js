import { CesiumZondy } from '../core/Base';
import MapGISM3D from './MapGISM3D';

const scratchPositionNormal = new Cesium.Cartesian3();
const scratchCartographic = new Cesium.Cartographic();
const scratchMatrix = new Cesium.Matrix4();
const scratchCenter = new Cesium.Cartesian3();
const scratchPosition = new Cesium.Cartesian3();
const scratchDirection = new Cesium.Cartesian3();

const scratchStack = [];
function unloadTile(tilesetParam, tile) {
    // debugger;
    const tileset = tilesetParam;
    tileset.tileUnload.raiseEvent(tile);
    tileset._statistics.decrementLoadCounts(tile.content);
    tileset._statistics.numberOfTilesWithContentReady -= 1;
    tile.unloadContent();
}

function destroyTile(tileset, tile) {
    tileset._cache.unloadTile(tileset, tile, unloadTile);
    tile.destroy();
}

function destroySubtree(tilesetParam, tileParam) {
    const tileset = tilesetParam;
    let tile = tileParam;
    const root = tile;
    const stack = scratchStack;
    stack.push(tile);
    while (stack.length > 0) {
        tile = stack.pop();
        const { children } = tile;
        const { length } = children;
        for (let i = 0; i < length; i += 1) {
            stack.push(children[i]);
        }
        if (tile !== root) {
            destroyTile(tileset, tile);
            tileset._statistics.numberOfTilesTotal += 1;
        }
    }
    root.children = [];
}

function addToProcessingQueue(tilesetParam, tile) {
    const tileset = tilesetParam;
    return () => {
        tileset._processingQueue.push(tile);

        tileset._statistics.numberOfPendingRequests -= 1;
        tileset._statistics.numberOfTilesProcessing += 1;
    };
}

function handleTileFailure(tilesetParam, tile) {
    const tileset = tilesetParam;
    return (error) => {
        if (tileset._processingQueue.indexOf(tile) >= 0) {
            // Failed during processing
            tileset._statistics.numberOfTilesProcessing -= 1;
        } else {
            // Failed when making request
            tileset._statistics.numberOfPendingRequests -= 1;
        }

        const { url } = tile._contentResource;
        const message = Cesium.defined(error.message) ? error.message : error.toString();
        if (tileset.tileFailed.numberOfListeners > 0) {
            tileset.tileFailed.raiseEvent({
                url,
                message
            });
        } else {
            // console.log(`m3d数据加载失败: ${url}`);
            // console.log(`错误: ${message}`);
        }
    };
}

function handleTileSuccess(tilesetParam, tile) {
    const tileset = tilesetParam;
    return () => {
        tileset._statistics.numberOfTilesProcessing -= 1;

        if (!tile.hasTilesetContent) {
            // RESEARCH_IDEA: ability to unload tiles (without content) for an
            // external tileset when all the tiles are unloaded.
            tileset._statistics.incrementLoadCounts(tile.content);
            tileset._statistics.numberOfTilesWithContentReady += 1;
            tileset._statistics.numberOfLoadedTilesTotal += 1;

            // Add to the tile cache. Previously expired tiles are already in the cache and won't get re-added.
            tileset._cache.add(tile);
        }

        tileset.tileLoad.raiseEvent(tile);
    };
}

function updateDynamicScreenSpaceError(tilesetParam, frameState) {
    const tileset = tilesetParam;
    let up;
    let direction;
    let height;
    let minimumHeight;
    let maximumHeight;

    const { camera } = frameState;
    const root = tileset._root;
    const tileBoundingVolume = root.contentBoundingVolume;

    if (tileBoundingVolume instanceof Cesium.TileBoundingRegion) {
        up = Cesium.Cartesian3.normalize(camera.positionWC, scratchPositionNormal);
        direction = camera.directionWC;
        height = camera.positionCartographic.height;
        minimumHeight = tileBoundingVolume.minimumHeight;
        maximumHeight = tileBoundingVolume.maximumHeight;
    } else {
        // Transform camera position and direction into the local coordinate system of the tileset
        const transformLocal = Cesium.Matrix4.inverseTransformation(root.computedTransform, scratchMatrix);
        const { ellipsoid } = frameState.mapProjection;
        const { boundingVolume } = tileBoundingVolume;
        const centerLocal = Cesium.Matrix4.multiplyByPoint(transformLocal, boundingVolume.center, scratchCenter);
        if (Cesium.Cartesian3.magnitude(centerLocal) > ellipsoid.minimumRadius) {
            // The tileset is defined in WGS84. Approximate the minimum and maximum height.
            const centerCartographic = Cesium.Cartographic.fromCartesian(centerLocal, ellipsoid, scratchCartographic);
            up = Cesium.Cartesian3.normalize(camera.positionWC, scratchPositionNormal);
            direction = camera.directionWC;
            height = camera.positionCartographic.height;
            minimumHeight = 0.0;
            maximumHeight = centerCartographic.height * 2.0;
        } else {
            // The tileset is defined in local coordinates (z-up)
            const positionLocal = Cesium.Matrix4.multiplyByPoint(transformLocal, camera.positionWC, scratchPosition);
            up = Cesium.Cartesian3.UNIT_Z;
            direction = Cesium.Matrix4.multiplyByPointAsVector(transformLocal, camera.directionWC, scratchDirection);
            direction = Cesium.Cartesian3.normalize(direction, direction);
            height = positionLocal.z;
            if (tileBoundingVolume instanceof Cesium.TileOrientedBoundingBox) {
                // Assuming z-up, the last component stores the half-height of the box
                const boxHeight = root._header.boundingVolume.box[11];
                minimumHeight = centerLocal.z - boxHeight;
                maximumHeight = centerLocal.z + boxHeight;
            } else if (tileBoundingVolume instanceof Cesium.TileBoundingSphere) {
                const { radius } = boundingVolume;
                minimumHeight = centerLocal.z - radius;
                maximumHeight = centerLocal.z + radius;
            }
        }
    }

    // The range where the density starts to lessen. Start at the quarter height of the tileset.
    const heightFalloff = tileset.dynamicScreenSpaceErrorHeightFalloff;
    const heightClose = minimumHeight + (maximumHeight - minimumHeight) * heightFalloff;
    const heightFar = maximumHeight;

    const t = Cesium.Math.clamp((height - heightClose) / (heightFar - heightClose), 0.0, 1.0);

    // Increase density as the camera tilts towards the horizon
    const dot = Math.abs(Cesium.Cartesian3.dot(direction, up));
    let horizonFactor = 1.0 - dot;

    // Weaken the horizon factor as the camera height increases, implying the camera is further away from the tileset.
    // The goal is to increase density for the "street view", not when viewing the tileset from a distance.
    horizonFactor *= 1.0 - t;

    let density = tileset.dynamicScreenSpaceErrorDensity;
    density *= horizonFactor;

    tileset._dynamicScreenSpaceErrorComputedDensity = density;
}

/// ////////////////////////////////////////////////////////////////////////

function requestContent(tileset, tile) {
    if (tile.hasEmptyContent) {
        return;
    }

    const statistics = tileset._statistics;
    const expired = tile.contentExpired;
    const requested = tile.requestContent();

    if (!requested) {
        statistics.numberOfAttemptedRequests += 1;
        return;
    }

    if (expired) {
        if (tile.hasTilesetContent) {
            destroySubtree(tileset, tile);
        } else {
            statistics.decrementLoadCounts(tile.content);
            statistics.numberOfTilesWithContentReady -= 1;
        }
    }

    statistics.numberOfPendingRequests += 1;
    tileset._requestedTilesInFlight.push(tile);

    tile.contentReadyToProcessPromise.then(addToProcessingQueue(tileset, tile));
    tile.contentReadyPromise.then(handleTileSuccess(tileset, tile)).otherwise(handleTileFailure(tileset, tile));
}

function sortRequestByPriority(a, b) {
    return a._priority - b._priority;
}

function cancelOutOfViewRequests(tileset, frameState) {
    const requestedTilesInFlight = tileset._requestedTilesInFlight;
    let removeCount = 0;
    const { length } = requestedTilesInFlight;
    for (let i = 0; i < length; i += 1) {
        const tile = requestedTilesInFlight[i];

        // NOTE: This is framerate dependant so make sure the threshold check is small
        const outOfView = frameState.frameNumber - tile._touchedFrame >= 1;
        if (tile._contentState !== Cesium.Cesium3DTileContentState.LOADING) {
            // No longer fetching from host, don't need to track it anymore. Gets marked as LOADING in Cesium3DTile::requestContent().
            removeCount += 1;
            // continue;
        } else if (outOfView) {
            // RequestScheduler will take care of cancelling it
            tile._request.cancel();
            removeCount += 1;
            // continue;
        } else if (removeCount > 0) {
            requestedTilesInFlight[i - removeCount] = tile;
        }
    }

    requestedTilesInFlight.length -= removeCount;
}

// eslint-disable-next-line no-unused-vars
function requestTiles(tileset, isAsync) {
    // Sort requests by priority before making any requests.
    // This makes it less likely that requests will be cancelled after being issued.
    const requestedTiles = tileset._requestedTiles;
    const { length } = requestedTiles;
    requestedTiles.sort(sortRequestByPriority);
    for (let i = 0; i < length; i += 1) {
        requestContent(tileset, requestedTiles[i]);
    }
}

function filterProcessingQueue(tileset) {
    const tiles = tileset._processingQueue;
    const { length } = tiles;

    let removeCount = 0;
    for (let i = 0; i < length; i += 1) {
        const tile = tiles[i];
        if (tile._contentState === Cesium.Cesium3DTileContentState.PROCESSING) {
            if (removeCount > 0) {
                tiles[i - removeCount] = tile;
            }
        } else {
            removeCount += 1;
        }
    }
    tiles.length -= removeCount;
}

function processTiles(tileset, frameState) {
    filterProcessingQueue(tileset);
    const tiles = tileset._processingQueue;
    const { length } = tiles;

    // Process tiles in the PROCESSING state so they will eventually move to the READY state.
    for (let i = 0; i < length; i += 1) {
        tiles[i].process(tileset, frameState);
    }
}

/// ////////////////////////////////////////////////////////////////////////

const scratchCartesian = new Cesium.Cartesian3();

const stringOptions = {
    maximumFractionDigits: 3
};

function formatMemoryString(memorySizeInBytes) {
    const memoryInMegabytes = memorySizeInBytes / 1048576;
    if (memoryInMegabytes < 1.0) {
        return memoryInMegabytes.toLocaleString(undefined, stringOptions);
    }
    return Math.round(memoryInMegabytes).toLocaleString();
}

function computeTileLabelPosition(tile) {
    const { boundingVolume } = tile.boundingVolume;
    const { halfAxes } = boundingVolume;
    const { radius } = boundingVolume;

    let position = Cesium.Cartesian3.clone(boundingVolume.center, scratchCartesian);
    if (Cesium.defined(halfAxes)) {
        position.x += 0.75 * (halfAxes[0] + halfAxes[3] + halfAxes[6]);
        position.y += 0.75 * (halfAxes[1] + halfAxes[4] + halfAxes[7]);
        position.z += 0.75 * (halfAxes[2] + halfAxes[5] + halfAxes[8]);
    } else if (Cesium.defined(radius)) {
        let normal = Cesium.Cartesian3.normalize(boundingVolume.center, scratchCartesian);
        normal = Cesium.Cartesian3.multiplyByScalar(normal, 0.75 * radius, scratchCartesian);
        position = Cesium.Cartesian3.add(normal, boundingVolume.center, scratchCartesian);
    }
    return position;
}

function addTileDebugLabel(tile, tileset, position) {
    let labelString = '';
    let attributes = 0;

    if (tileset.debugShowGeometricError) {
        labelString += `\nGeometric error: ${tile.geometricError}`;
        attributes += 1;
    }

    if (tileset.debugShowRenderingStatistics) {
        labelString += `\nCommands: ${tile.commandsLength}`;
        attributes += 1;

        // Don't display number of points or triangles if 0.
        const numberOfPoints = tile.content.pointsLength;
        if (numberOfPoints > 0) {
            labelString += `\nPoints: ${tile.content.pointsLength}`;
            attributes += 1;
        }

        const numberOfTriangles = tile.content.trianglesLength;
        if (numberOfTriangles > 0) {
            labelString += `\nTriangles: ${tile.content.trianglesLength}`;
            attributes += 1;
        }

        labelString += `\nFeatures: ${tile.content.featuresLength}`;
        attributes += 1;
    }

    if (tileset.debugShowMemoryUsage) {
        labelString += `\n纹理内存: ${formatMemoryString(tile.content.texturesByteLength)}`;
        labelString += `\n几何内存: ${formatMemoryString(tile.content.geometryByteLength)}`;
        attributes += 2;
    }

    if (tileset.debugShowUrl) {
        labelString += `\nUrl: ${tile._header.content.uri}`;
        attributes += 1;
    }

    const newLabel = {
        text: labelString.substring(1),
        position,
        font: `${19 - attributes}px sans-serif`,
        showBackground: true,
        disableDepthTestDistance: Number.POSITIVE_INFINITY
    };

    return tileset._tileDebugLabels.add(newLabel);
}

function updateTileDebugLabels(tileset, frameState) {
    let i;
    let tile;
    const selectedTiles = tileset._selectedTiles;
    const selectedLength = selectedTiles.length;
    const emptyTiles = tileset._emptyTiles;
    const emptyLength = emptyTiles.length;
    tileset._tileDebugLabels.removeAll();

    if (tileset.debugPickedTileLabelOnly) {
        if (Cesium.defined(tileset.debugPickedTile)) {
            const position = Cesium.defined(tileset.debugPickPosition)
                ? tileset.debugPickPosition
                : computeTileLabelPosition(tileset.debugPickedTile);
            const label = addTileDebugLabel(tileset.debugPickedTile, tileset, position);
            label.pixelOffset = new Cesium.Cartesian2(15, -15); // Offset to avoid picking the label.
        }
    } else {
        for (i = 0; i < selectedLength; i += 1) {
            tile = selectedTiles[i];
            addTileDebugLabel(tile, tileset, computeTileLabelPosition(tile));
        }
        for (i = 0; i < emptyLength; i += 1) {
            tile = emptyTiles[i];
            if (tile.hasTilesetContent) {
                addTileDebugLabel(tile, tileset, computeTileLabelPosition(tile));
            }
        }
    }
    tileset._tileDebugLabels.update(frameState);
}

function updateTiles(tilesetParam, frameState, isRender) {
    const tileset = tilesetParam;
    tileset._styleEngine.applyStyle(tileset, frameState);

    const statistics = tileset._statistics;
    const { commandList } = frameState;
    const numberOfInitialCommands = commandList.length;
    const selectedTiles = tileset._selectedTiles;
    const selectedLength = selectedTiles.length;
    const emptyTiles = tileset._emptyTiles;
    const emptyLength = emptyTiles.length;
    const { tileVisible } = tileset;
    let i;
    let tile;

    const bivariateVisibilityTest =
        tileset._skipLevelOfDetail &&
        tileset._hasMixedContent &&
        frameState.context.stencilBuffer &&
        selectedLength > 0;

    tileset._backfaceCommands.length = 0;

    if (bivariateVisibilityTest) {
        if (!Cesium.defined(tileset._stencilClearCommand)) {
            tileset._stencilClearCommand = new Cesium.ClearCommand({
                stencil: 0,
                pass: Cesium.Pass.CESIUM_3D_TILE,
                renderState: Cesium.RenderState.fromCache({
                    stencilMask: Cesium.StencilConstants.SKIP_LOD_MASK
                })
            });
        }
        commandList.push(tileset._stencilClearCommand);
    }

    const lengthBeforeUpdate = commandList.length;
    for (i = 0; i < selectedLength; i += 1) {
        tile = selectedTiles[i];
        // Raise the tileVisible event before update in case the tileVisible event
        // handler makes changes that update needs to apply to WebGL resources
        if (isRender) {
            tileVisible.raiseEvent(tile);
        }
        // begin判断进行偏移
        //  let bd = tile.boundingSphere;
        //  let center = bd.center;
        //  let baozaoDistance = 400;
        //  let  fDetal = (selectedLength - 1)*baozaoDistance *0.8;
        //  let fZTrans = (selectedLength - i - 1)*baozaoDistance - fDetal;//最后一个没有移动
        //  tile.transform = Matrix4.setTranslation(tile.transform,new Cartesian3(0,0,fZTrans),tile.transform);

        // end
        tile.update(tileset, frameState);
        statistics.incrementSelectionCounts(tile.content);
        statistics.selected += 1;
    }
    for (i = 0; i < emptyLength; i += 1) {
        tile = emptyTiles[i];
        tile.update(tileset, frameState);
    }

    let addedCommandsLength = commandList.length - lengthBeforeUpdate;

    tileset._backfaceCommands.trim();

    if (bivariateVisibilityTest) {
        /**
         * Consider 'effective leaf' tiles as selected tiles that have no selected descendants. They may have children,
         * but they are currently our effective leaves because they do not have selected descendants. These tiles
         * are those where with tile._finalResolution === true.
         * Let 'unresolved' tiles be those with tile._finalResolution === false.
         *
         * 1. Render just the backfaces of unresolved tiles in order to lay down z
         * 2. Render all frontfaces wherever tile._selectionDepth > stencilBuffer.
         *    Replace stencilBuffer with tile._selectionDepth, when passing the z test.
         *    Because children are always drawn before ancestors {@link Cesium3DTilesetTraversal#traverseAndSelect},
         *    this effectively draws children first and does not draw ancestors if a descendant has already
         *    been drawn at that pixel.
         *    Step 1 prevents child tiles from appearing on top when they are truly behind ancestor content.
         *    If they are behind the backfaces of the ancestor, then they will not be drawn.
         *
         * NOTE: Step 2 sometimes causes visual artifacts when backfacing child content has some faces that
         * partially face the camera and are inside of the ancestor content. Because they are inside, they will
         * not be culled by the depth writes in Step 1, and because they partially face the camera, the stencil tests
         * will draw them on top of the ancestor content.
         *
         * NOTE: Because we always render backfaces of unresolved tiles, if the camera is looking at the backfaces
         * of an object, they will always be drawn while loading, even if backface culling is enabled.
         */

        const backfaceCommands = tileset._backfaceCommands.values;
        const backfaceCommandsLength = backfaceCommands.length;

        commandList.length += backfaceCommandsLength;

        // copy commands to the back of the commandList
        for (i = addedCommandsLength - 1; i >= 0; i -= 1) {
            commandList[lengthBeforeUpdate + backfaceCommandsLength + i] = commandList[lengthBeforeUpdate + i];
        }

        // move backface commands to the front of the commandList
        for (i = 0; i < backfaceCommandsLength; i += 1) {
            commandList[lengthBeforeUpdate + i] = backfaceCommands[i];
        }
    }

    // Number of commands added by each update above
    addedCommandsLength = commandList.length - numberOfInitialCommands;
    statistics.numberOfCommands = addedCommandsLength;

    // Only run EDL if simple attenuation is on
    if (
        isRender &&
        tileset.pointCloudShading.attenuation &&
        tileset.pointCloudShading.eyeDomeLighting &&
        addedCommandsLength > 0
    ) {
        tileset._pointCloudEyeDomeLighting.update(frameState, numberOfInitialCommands, tileset.pointCloudShading);
    }

    if (isRender) {
        if (
            tileset.debugShowGeometricError ||
            tileset.debugShowRenderingStatistics ||
            tileset.debugShowMemoryUsage ||
            tileset.debugShowUrl
        ) {
            if (!Cesium.defined(tileset._tileDebugLabels)) {
                tileset._tileDebugLabels = new Cesium.LabelCollection();
            }
            updateTileDebugLabels(tileset, frameState);
        } else {
            tileset._tileDebugLabels = tileset._tileDebugLabels && tileset._tileDebugLabels.destroy();
        }
    }
}

/// ////////////////////////////////////////////////////////////////////////

function raiseLoadProgressEvent(tilesetParam, frameState) {
    const tileset = tilesetParam;
    const statistics = tileset._statistics;
    const statisticsLast = tileset._statisticsLast;

    const { numberOfPendingRequests } = statistics;
    const { numberOfTilesProcessing } = statistics;
    const lastNumberOfPendingRequest = statisticsLast.numberOfPendingRequests;
    const lastNumberOfTilesProcessing = statisticsLast.numberOfTilesProcessing;

    Cesium.Cesium3DTilesetStatistics.clone(statistics, statisticsLast);

    const progressChanged =
        numberOfPendingRequests !== lastNumberOfPendingRequest ||
        numberOfTilesProcessing !== lastNumberOfTilesProcessing;

    if (progressChanged) {
        frameState.afterRender.push(() => {
            tileset.loadProgress.raiseEvent(numberOfPendingRequests, numberOfTilesProcessing);
        });
    }

    tileset._tilesLoaded =
        statistics.numberOfPendingRequests === 0 &&
        statistics.numberOfTilesProcessing === 0 &&
        statistics.numberOfAttemptedRequests === 0;

    // Events are raised (added to the afterRender queue) here since promises
    // may resolve outside of the update loop that then raise events, e.g.,
    // model's readyPromise.
    if (progressChanged && tileset._tilesLoaded) {
        frameState.afterRender.push(() => {
            tileset.allTilesLoaded.raiseEvent();
        });
        if (!tileset._initialTilesLoaded) {
            tileset._initialTilesLoaded = true;
            frameState.afterRender.push(() => {
                tileset.initialTilesLoaded.raiseEvent();
            });
        }
    }
}

function resetMinimumMaximum(tilesetParam) {
    const tileset = tilesetParam;
    tileset._heatmap.resetMinimumMaximum();
    tileset._minimumPriority.depth = Number.MAX_VALUE;
    tileset._maximumPriority.depth = -Number.MAX_VALUE;
    tileset._minimumPriority.foveatedFactor = Number.MAX_VALUE;
    tileset._maximumPriority.foveatedFactor = -Number.MAX_VALUE;
    tileset._minimumPriority.distance = Number.MAX_VALUE;
    tileset._maximumPriority.distance = -Number.MAX_VALUE;
    tileset._minimumPriority.reverseScreenSpaceError = Number.MAX_VALUE;
    tileset._maximumPriority.reverseScreenSpaceError = -Number.MAX_VALUE;
}

/**
 * Called when {@link Viewer} or {@link CesiumWidget} render the scene to
 * get the draw commands needed to render this primitive.
 * <p>
 * Do not call this function directly.  This is documented just to
 * list the exceptions that may be propagated when the scene is rendered:
 * </p>
 */
function update(tilesetParam, frameState, passStatistics, passOptions) {
    const tileset = tilesetParam;

    if (frameState.mode === Cesium.SceneMode.MORPHING) {
        return false;
    }

    if (!tileset.ready) {
        return false;
    }

    const statistics = tileset._statistics;
    statistics.clear();

    const { isRender } = passOptions;

    // Resets the visibility check for each pass
    tileset._updatedVisibilityFrame += 1;

    // Update any tracked min max values
    resetMinimumMaximum(tileset);

    const ready = passOptions.traversal.selectTiles(tileset, frameState);

    if (passOptions.requestTiles) {
        requestTiles(tileset);
    }

    updateTiles(tileset, frameState, isRender);

    // Update pass statistics
    Cesium.Cesium3DTilesetStatistics.clone(statistics, passStatistics);

    if (isRender) {
        const credits = tileset._credits;
        if (Cesium.defined(credits) && statistics.selected !== 0) {
            const { length } = credits;
            for (let i = 0; i < length; i += 1) {
                frameState.creditDisplay.addCredit(credits[i]);
            }
        }
    }

    return ready;
}

function initPlanishPolygon(m3dSetParam) {
    const m3dSet = m3dSetParam;
    if (m3dSet._planishPolygons.length === 0) {
        return;
    }
    if (!Cesium.defined(m3dSet._geometryDepth)) {
        m3dSet._geometryDepth = new Cesium.GeometryDepth();
    }
    m3dSet._geometryDepth._polygons = m3dSet._planishPolygons;
}

// -->>

export default class MapGISM3DSet {
    constructor(options) {
        const optionsParam = Cesium.defaultValue(options, Cesium.defaultValue.EMPTY_OBJECT);

        // >>includeStart('debug', pragmas.debug);
        Cesium.Check.defined('options.url', optionsParam.url);
        // >>includeEnd('debug');
        this._url = undefined;
        this._basePath = undefined;
        this._root = undefined;
        this._asset = undefined; // Metadata for the entire tileset
        this._properties = undefined; // Metadata for per-model/point/etc properties
        this._geometricError = undefined; // Geometric error when the tree is not rendered at all
        this._extensionsUsed = undefined;
        this._gltfUpAxis = undefined;
        this._cache = new Cesium.Cesium3DTilesetCache();
        this._processingQueue = [];
        this._selectedTiles = [];
        this._emptyTiles = [];
        this._requestedTiles = [];
        this._selectedTilesToStyle = [];
        this._loadTimestamp = undefined;
        this._timeSinceLoad = 0.0;
        // 是否是IGserver_isIGserver= optionsParam.igserver;
        this._isIGServer = Cesium.defaultValue(optionsParam.igserver, false);
        // 以下内容 需要再docinfo返回值得到之后才能得到。
        this._layerRenderIndex = Cesium.defaultValue(optionsParam.layerRenderIndex, 0); // 渲染图层索引
        this._layerIndex = Cesium.defaultValue(optionsParam.layerIndex, 0); // 图层索引(用来标识组图层的概念)
        this._gdbpUrl = Cesium.defaultValue(optionsParam.gdbpUrl, ''); // 原始数据在gdb中的数据
        // hys添加名称和Id的记录 一下内容只有去到数据才能获取的到
        this._name = undefined; // 图层名
        this._guid = undefined; // 图层的guid
        this._extLayer = undefined; // 这里先为 挂接单体化预留 用来绑定单体化所用到的附加图层
        this._updatedVisibilityFrame = 0;
        this._extras = undefined;
        this._credits = undefined;
        this._planishPolygons = []; // 压平区

        this._cullWithChildrenBounds = Cesium.defaultValue(optionsParam.cullWithChildrenBounds, true);
        this._allTilesAdditive = true;

        this._hasMixedContent = false;

        this._stencilClearCommand = undefined;
        this._backfaceCommands = new Cesium.ManagedArray();

        this._maximumScreenSpaceError = Cesium.defaultValue(optionsParam.maximumScreenSpaceError, 16);
        this._maximumMemoryUsage = Cesium.defaultValue(optionsParam.maximumMemoryUsage, 512);

        this._styleEngine = new Cesium.Cesium3DTileStyleEngine();

        this._modelMatrix = Cesium.defined(optionsParam.modelMatrix)
            ? Cesium.Matrix4.clone(optionsParam.modelMatrix)
            : Cesium.Matrix4.clone(Cesium.Matrix4.IDENTITY);

        this._statistics = new Cesium.Cesium3DTilesetStatistics();
        this._statisticsLast = new Cesium.Cesium3DTilesetStatistics();
        this._statisticsPerPass = new Array(Cesium.Cesium3DTilePass.NUMBER_OF_PASSES);

        for (let i = 0; i < Cesium.Cesium3DTilePass.NUMBER_OF_PASSES; i += 1) {
            this._statisticsPerPass[i] = new Cesium.Cesium3DTilesetStatistics();
        }

        this._requestedTilesInFlight = [];

        this._maximumPriority = {
            foveatedFactor: -Number.MAX_VALUE,
            depth: -Number.MAX_VALUE,
            distance: -Number.MAX_VALUE,
            reverseScreenSpaceError: -Number.MAX_VALUE
        };
        this._minimumPriority = {
            foveatedFactor: Number.MAX_VALUE,
            depth: Number.MAX_VALUE,
            distance: Number.MAX_VALUE,
            reverseScreenSpaceError: Number.MAX_VALUE
        };
        this._heatmap = new Cesium.Cesium3DTilesetHeatmap(optionsParam.debugHeatmapTilePropertyName);

        /**
         * Optimization option. Don't request tiles that will likely be unused when they come back because of the camera's movement.
         *
         * @type {Boolean}
         * @default true
         */
        this.cullRequestsWhileMoving = Cesium.defaultValue(optionsParam.cullRequestsWhileMoving, true);

        /**
         * Optimization option. Multiplier used in culling requests while moving. Larger is more aggressive culling, smaller less aggressive culling.
         *
         * @type {Number}
         * @default 60.0
         */
        this.cullRequestsWhileMovingMultiplier = Cesium.defaultValue(
            optionsParam.cullRequestsWhileMovingMultiplier,
            60.0
        );

        /**
         * Optimization option. If between (0.0, 0.5], tiles at or above the screen space error for the reduced screen resolution of <code>progressiveResolutionHeightFraction*screenHeight</code> will be prioritized first. This can help get a quick layer of tiles down while full resolution tiles continue to load.
         *
         * @type {Number}
         * @default 0.3
         */
        this.progressiveResolutionHeightFraction = Cesium.Math.clamp(
            Cesium.defaultValue(optionsParam.progressiveResolutionHeightFraction, 0.3),
            0.0,
            0.5
        );

        /**
         * Optimization option. Prefer loading of leaves first.
         *
         * @type {Boolean}
         * @default false
         */
        this.preferLeaves = Cesium.defaultValue(optionsParam.preferLeaves, false);

        this._tilesLoaded = false;
        this._initialTilesLoaded = false;

        this._tileDebugLabels = undefined;

        this._readyPromise = Cesium.when.defer();

        this._classificationType = optionsParam.classificationType;

        this._ellipsoid = Cesium.defaultValue(optionsParam.ellipsoid, Cesium.Ellipsoid.WGS84);

        this._initialClippingPlanesOriginMatrix = Cesium.Matrix4.IDENTITY; // Computed from the tileset JSON.
        this._clippingPlanesOriginMatrix = undefined; // Combines the above with any run-time transforms.
        this._clippingPlanesOriginMatrixDirty = true;

        /**
         * Preload tiles when <code>tileset.show</code> is <code>false</code>. Loads tiles as if the tileset is visible but does not render them.
         *
         * @type {Boolean}
         * @default false
         */
        this.preloadWhenHidden = Cesium.defaultValue(optionsParam.preloadWhenHidden, false);

        /**
         * Optimization option. Fetch tiles at the camera's flight destination while the camera is in flight.
         *
         * @type {Boolean}
         * @default true
         */
        this.preloadFlightDestinations = Cesium.defaultValue(optionsParam.preloadFlightDestinations, true);
        this._pass = undefined; // Cesium3DTilePass

        /**
         * Optimization option. Whether the tileset should refine based on a dynamic screen space error. Tiles that are further
         * away will be rendered with lower detail than closer tiles. This improves performance by rendering fewer
         * tiles and making less requests, but may result in a slight drop in visual quality for tiles in the distance.
         * The algorithm is biased towards "street views" where the camera is close to the ground plane of the tileset and looking
         * at the horizon. In addition results are more accurate for tightly fitting bounding volumes like box and region.
         *
         * @type {Boolean}
         * @default false
         */
        this.dynamicScreenSpaceError = Cesium.defaultValue(optionsParam.dynamicScreenSpaceError, false);

        /**
         * Optimization option. Prioritize loading tiles in the center of the screen by temporarily raising the
         * screen space error for tiles around the edge of the screen. Screen space error returns to normal once all
         * the tiles in the center of the screen as determined by the {@link Cesium3DTileset#foveatedConeSize} are loaded.
         *
         * @type {Boolean}
         * @default true
         */
        this.foveatedScreenSpaceError = Cesium.defaultValue(optionsParam.foveatedScreenSpaceError, true);
        this._foveatedConeSize = Cesium.defaultValue(optionsParam.foveatedConeSize, 0.1);
        this._foveatedMinimumScreenSpaceErrorRelaxation = Cesium.defaultValue(
            optionsParam.foveatedMinimumScreenSpaceErrorRelaxation,
            0.0
        );

        /**
         * Gets a function that will update the foveated screen space error for a tile.
         *
         * @type {Cesium3DTileset~foveatedInterpolationCallback} A callback to control how much to raise the screen space error for tiles outside the foveated cone, interpolating between {@link Cesium3DTileset#foveatedMinimumScreenSpaceErrorRelaxation} and {@link Cesium3DTileset#maximumScreenSpaceError}.
         */
        this.foveatedInterpolationCallback = Cesium.defaultValue(
            optionsParam.foveatedInterpolationCallback,
            Cesium.Math.lerp
        );

        /**
         * Optimization option. Used when {@link Cesium3DTileset#foveatedScreenSpaceError} is true to control
         * how long in seconds to wait after the camera stops moving before deferred tiles start loading in.
         * This time delay prevents requesting tiles around the edges of the screen when the camera is moving.
         * Setting this to 0.0 will immediately request all tiles in any given view.
         *
         * @type {Number}
         * @default 0.2
         */
        this.foveatedTimeDelay = Cesium.defaultValue(optionsParam.foveatedTimeDelay, 0.2);

        /**
         * A scalar that determines the density used to adjust the dynamic screen space error, similar to {@link Fog}. Increasing this
         * value has the effect of increasing the maximum screen space error for all tiles, but in a non-linear fashion.
         * The error starts at 0.0 and increases exponentially until a midpoint is reached, and then approaches 1.0 asymptotically.
         * This has the effect of keeping high detail in the closer tiles and lower detail in the further tiles, with all tiles
         * beyond a certain distance all roughly having an error of 1.0.
         * <p>
         * The dynamic error is in the range [0.0, 1.0) and is multiplied by <code>dynamicScreenSpaceErrorFactor</code> to produce the
         * final dynamic error. This dynamic error is then subtracted from the tile's actual screen space error.
         * </p>
         * <p>
         * Increasing <code>dynamicScreenSpaceErrorDensity</code> has the effect of moving the error midpoint closer to the camera.
         * It is analogous to moving fog closer to the camera.
         * </p>
         *
         * @type {Number}
         * @default 0.00278
         */
        this.dynamicScreenSpaceErrorDensity = 0.00278;

        /**
         * A factor used to increase the screen space error of tiles for dynamic screen space error. As this value increases less tiles
         * are requested for rendering and tiles in the distance will have lower detail. If set to zero, the feature will be disabled.
         *
         * @type {Number}
         * @default 4.0
         */
        this.dynamicScreenSpaceErrorFactor = 4.0;

        /**
         * A ratio of the tileset's height at which the density starts to falloff. If the camera is below this height the
         * full computed density is applied, otherwise the density falls off. This has the effect of higher density at
         * street level views.
         * <p>
         * Valid values are between 0.0 and 1.0.
         * </p>
         *
         * @type {Number}
         * @default 0.25
         */
        this.dynamicScreenSpaceErrorHeightFalloff = 0.25;

        this._dynamicScreenSpaceErrorComputedDensity = 0.0; // Updated based on the camera position and direction

        /**
         * Determines whether the tileset casts or receives shadows from each light source.
         * <p>
         * Enabling shadows has a performance impact. A tileset that casts shadows must be rendered twice, once from the camera and again from the light's point of view.
         * </p>
         * <p>
         * Shadows are rendered only when {@link Viewer#shadows} is <code>true</code>.
         * </p>
         *
         * @type {ShadowMode}
         * @default ShadowMode.ENABLED
         */
        this.shadows = Cesium.defaultValue(optionsParam.shadows, Cesium.ShadowMode.ENABLED);

        /**
         * Determines if the tileset will be shown.
         *
         * @type {Boolean}
         * @default true
         */
        this.show = Cesium.defaultValue(optionsParam.show, true);

        /**
         * Defines how per-feature colors set from the Cesium API or declarative styling blend with the source colors from
         * the original feature, e.g. glTF material or per-point color in the tile.
         *
         * @type {Cesium3DTileColorBlendMode}
         * @default Cesium3DTileColorBlendMode.HIGHLIGHT
         */
        this.colorBlendMode = Cesium.Cesium3DTileColorBlendMode.HIGHLIGHT;

        /**
         * Defines the value used to linearly interpolate between the source color and feature color when the {@link Cesium3DTileset#colorBlendMode} is <code>MIX</code>.
         * A value of 0.0 results in the source color while a value of 1.0 results in the feature color, with any value in-between
         * resulting in a mix of the source color and feature color.
         *
         * @type {Number}
         * @default 0.5
         */
        this.colorBlendAmount = 0.5;

        /**
         * Options for controlling point size based on geometric error and eye dome lighting.
         * @type {PointCloudShading}
         */
        this.pointCloudShading = new Cesium.PointCloudShading(optionsParam.pointCloudShading);

        this._pointCloudEyeDomeLighting = new Cesium.PointCloudEyeDomeLighting();

        /**
         * The event fired to indicate progress of loading new tiles.  This event is fired when a new tile
         * is requested, when a requested tile is finished downloading, and when a downloaded tile has been
         * processed and is ready to render.
         * <p>
         * The number of pending tile requests, <code>numberOfPendingRequests</code>, and number of tiles
         * processing, <code>numberOfTilesProcessing</code> are passed to the event listener.
         * </p>
         * <p>
         * This event is fired at the end of the frame after the scene is rendered.
         * </p>
         *
         * @type {Event}
         * @default new Event()
         *
         * @example
         * tileset.loadProgress.addEventListener(function(numberOfPendingRequests, numberOfTilesProcessing) {
         *     if ((numberOfPendingRequests === 0) && (numberOfTilesProcessing === 0)) {
         *         console.log('Stopped loading');
         *         return;
         *     }
         *
         *     console.log('Loading: requests: ' + numberOfPendingRequests + ', processing: ' + numberOfTilesProcessing);
         * });
         */
        this.loadProgress = new Cesium.Event();

        /**
         * The event fired to indicate that all tiles that meet the screen space error this frame are loaded. The tileset
         * is completely loaded for this view.
         * <p>
         * This event is fired at the end of the frame after the scene is rendered.
         * </p>
         *
         * @type {Event}
         * @default new Event()
         *
         * @example
         * tileset.allTilesLoaded.addEventListener(function() {
         *     console.log('All tiles are loaded');
         * });
         *
         * @see MapGISM3DSet#tilesLoaded
         */
        this.allTilesLoaded = new Cesium.Event();

        /**
         * The event fired to indicate that all tiles that meet the screen space error this frame are loaded. This event
         * is fired once when all tiles in the initial view are loaded.
         * <p>
         * This event is fired at the end of the frame after the scene is rendered.
         * </p>
         *
         * @type {Event}
         * @default new Event()
         *
         * @example
         * tileset.initialTilesLoaded.addEventListener(function() {
         *     console.log('Initial tiles are loaded');
         * });
         *
         * @see MapGISM3DSet#allTilesLoaded
         */
        this.initialTilesLoaded = new Cesium.Event();

        /**
         * The event fired to indicate that a tile's content was loaded.
         * <p>
         * The loaded {@link Cesium3DTile} is passed to the event listener.
         * </p>
         * <p>
         * This event is fired during the tileset traversal while the frame is being rendered
         * so that updates to the tile take effect in the same frame.  Do not create or modify
         * Cesium entities or primitives during the event listener.
         * </p>
         *
         * @type {Event}
         * @default new Event()
         *
         * @example
         * tileset.tileLoad.addEventListener(function(tile) {
         *     console.log('A tile was loaded.');
         * });
         */
        this.tileLoad = new Cesium.Event();

        /**
         * The event fired to indicate that a tile's content was unloaded.
         * <p>
         * The unloaded {@link Cesium3DTile} is passed to the event listener.
         * </p>
         * <p>
         * This event is fired immediately before the tile's content is unloaded while the frame is being
         * rendered so that the event listener has access to the tile's content.  Do not create
         * or modify Cesium entities or primitives during the event listener.
         * </p>
         *
         * @type {Event}
         * @default new Event()
         *
         * @example
         * tileset.tileUnload.addEventListener(function(tile) {
         *     console.log('A tile was unloaded from the cache.');
         * });
         *
         * @see MapGISM3DSet#maximumMemoryUsage
         * @see MapGISM3DSet#trimLoadedTiles
         */
        this.tileUnload = new Cesium.Event();

        /**
         * The event fired to indicate that a tile's content failed to load.
         * <p>
         * If there are no event listeners, error messages will be logged to the console.
         * </p>
         * <p>
         * The error object passed to the listener contains two properties:
         * <ul>
         * <li><code>url</code>: the url of the failed tile.</li>
         * <li><code>message</code>: the error message.</li>
         * </ul>
         *
         * @type {Event}
         * @default new Event()
         *
         * @example
         * tileset.tileFailed.addEventListener(function(error) {
         *     console.log('An error occurred loading tile: ' + error.url);
         *     console.log('Error: ' + error.message);
         * });
         */
        this.tileFailed = new Cesium.Event();

        /**
         * This event fires once for each visible tile in a frame.  This can be used to manually
         * style a tileset.
         * <p>
         * The visible {@link Cesium3DTile} is passed to the event listener.
         * </p>
         * <p>
         * This event is fired during the tileset traversal while the frame is being rendered
         * so that updates to the tile take effect in the same frame.  Do not create or modify
         * Cesium entities or primitives during the event listener.
         * </p>
         *
         * @type {Event}
         * @default new Event()
         *
         * @example
         * tileset.tileVisible.addEventListener(function(tile) {
         *     if (tile.content instanceof Cesium.Batched3DModel3DTileContent) {
         *         console.log('A Batched 3D Model tile is visible.');
         *     }
         * });
         *
         * @example
         * // Apply a red style and then manually set random colors for every other feature when the tile becomes visible.
         * tileset.style = new Cesium.Cesium3DTileStyle({
         *     color : 'color("red")'
         * });
         * tileset.tileVisible.addEventListener(function(tile) {
         *     let content = tile.content;
         *     let featuresLength = content.featuresLength;
         *     for (let i = 0; i < featuresLength; i+=2) {
         *         content.getFeature(i).color = Cesium.Color.fromRandom();
         *     }
         * });
         */
        this.tileVisible = new Cesium.Event();

        /**
         * Optimization option. Determines if level of detail skipping should be applied during the traversal.
         * <p>
         * The common strategy for replacement-refinement traversal is to store all levels of the tree in memory and require
         * all children to be loaded before the parent can refine. With this optimization levels of the tree can be skipped
         * entirely and children can be rendered alongside their parents. The tileset requires significantly less memory when
         * using this optimization.
         * </p>
         *
         * @type {Boolean}
         * @default true
         */
        this.skipLevelOfDetail = Cesium.defaultValue(optionsParam.skipLevelOfDetail, true);
        this._skipLevelOfDetail = this.skipLevelOfDetail;
        this._disableSkipLevelOfDetail = false;

        /**
         * The screen space error that must be reached before skipping levels of detail.
         * <p>
         * Only used when {@link MapGISM3DSet#skipLevelOfDetail} is <code>true</code>.
         * </p>
         *
         * @type {Number}
         * @default 1024
         */
        this.baseScreenSpaceError = Cesium.defaultValue(optionsParam.baseScreenSpaceError, 1024);

        /**
         * Multiplier defining the minimum screen space error to skip.
         * For example, if a tile has screen space error of 100, no tiles will be loaded unless they
         * are leaves or have a screen space error <code><= 100 / skipScreenSpaceErrorFactor</code>.
         * <p>
         * Only used when {@link MapGISM3DSet#skipLevelOfDetail} is <code>true</code>.
         * </p>
         *
         * @type {Number}
         * @default 16
         */
        this.skipScreenSpaceErrorFactor = Cesium.defaultValue(optionsParam.skipScreenSpaceErrorFactor, 16);

        /**
         * Constant defining the minimum number of levels to skip when loading tiles. When it is 0, no levels are skipped.
         * For example, if a tile is level 1, no tiles will be loaded unless they are at level greater than 2.
         * <p>
         * Only used when {@link MapGISM3DSet#skipLevelOfDetail} is <code>true</code>.
         * </p>
         *
         * @type {Number}
         * @default 1
         */
        this.skipLevels = Cesium.defaultValue(optionsParam.skipLevels, 1);

        /**
         * When true, only tiles that meet the maximum screen space error will ever be downloaded.
         * Skipping factors are ignored and just the desired tiles are loaded.
         * <p>
         * Only used when {@link MapGISM3DSet#skipLevelOfDetail} is <code>true</code>.
         * </p>
         *
         * @type {Boolean}
         * @default false
         */
        this.immediatelyLoadDesiredLevelOfDetail = Cesium.defaultValue(
            optionsParam.immediatelyLoadDesiredLevelOfDetail,
            false
        );

        /**
         * Determines whether siblings of visible tiles are always downloaded during traversal.
         * This may be useful for ensuring that tiles are already available when the viewer turns left/right.
         * <p>
         * Only used when {@link MapGISM3DSet#skipLevelOfDetail} is <code>true</code>.
         * </p>
         *
         * @type {Boolean}
         * @default false
         */
        // this.loadSiblings = Cesium.defaultValue(optionsParam.loadSiblings, false);

        // this._clippingPlanes = undefined;
        // this.clippingPlanes = optionsParam.clippingPlanes;

        // this._imageBasedLightingFactor = new Cesium.Cartesian2(1.0, 1.0);
        // Cesium.Cartesian2.clone(optionsParam.imageBasedLightingFactor, this._imageBasedLightingFactor);

        /**
         * The color and intensity of the sunlight used to shade a model.
         * <p>
         * For example, disabling additional light sources by setting <code>model.imageBasedLightingFactor = new Cartesian2(0.0, 0.0)</code> will make the
         * model much darker. Here, increasing the intensity of the light source will make the model brighter.
         * </p>
         *
         * @type {Cartesian3}
         * @default undefined
         */
        // this.lightColor = optionsParam.lightColor;

        /**
         * The sun's luminance at the zenith in kilo candela per meter squared to use for this model's procedural environment map.
         * This is used when {@link Cesium3DTileset#specularEnvironmentMaps} and {@link Cesium3DTileset#sphericalHarmonicCoefficients} are not defined.
         *
         * @type Number
         *
         * @default 0.5
         *
         */
        // this.luminanceAtZenith = Cesium.defaultValue(optionsParam.luminanceAtZenith, 0.5);

        /**
         * The third order spherical harmonic coefficients used for the diffuse color of image-based lighting. When <code>undefined</code>, a diffuse irradiance
         * computed from the atmosphere color is used.
         * <p>
         * There are nine <code>Cartesian3</code> coefficients.
         * The order of the coefficients is: L<sub>00</sub>, L<sub>1-1</sub>, L<sub>10</sub>, L<sub>11</sub>, L<sub>2-2</sub>, L<sub>2-1</sub>, L<sub>20</sub>, L<sub>21</sub>, L<sub>22</sub>
         * </p>
         *
         * These values can be obtained by preprocessing the environment map using the <code>cmgen</code> tool of
         * {@link https://github.com/google/filament/releases | Google's Filament project}. This will also generate a KTX file that can be
         * supplied to {@link Cesium3DTileset#specularEnvironmentMaps}.
         *
         * @type {Cartesian3[]}
         * @demo {@link https://cesiumjs.org/Cesium/Apps/Sandcastle/index.html?src=Image-Based Lighting.html|Sandcastle Image Based Lighting Demo}
         * @see {@link https://graphics.stanford.edu/papers/envmap/envmap.pdf|An Efficient Representation for Irradiance Environment Maps}
         */
        // this.sphericalHarmonicCoefficients = optionsParam.sphericalHarmonicCoefficients;

        /**
         * A URL to a KTX file that contains a cube map of the specular lighting and the convoluted specular mipmaps.
         *
         * @demo {@link https://cesiumjs.org/Cesium/Apps/Sandcastle/index.html?src=Image-Based Lighting.html|Sandcastle Image Based Lighting Demo}
         * @type {String}
         * @see Cesium3DTileset#sphericalHarmonicCoefficients
         */
        // this.specularEnvironmentMaps = optionsParam.specularEnvironmentMaps;

        /**
         * This property is for debugging only; it is not optimized for production use.
         * <p>
         * Determines if only the tiles from last frame should be used for rendering.  This
         * effectively "freezes" the tileset to the previous frame so it is possible to zoom
         * out and see what was rendered.
         * </p>
         *
         * @type {Boolean}
         * @default false
         */
        // this.debugFreezeFrame = Cesium.defaultValue(optionsParam.debugFreezeFrame, false);

        /**
         * This property is for debugging only; it is not optimized for production use.
         * <p>
         * When true, assigns a random color to each tile.  This is useful for visualizing
         * what features belong to what tiles, especially with additive refinement where features
         * from parent tiles may be interleaved with features from child tiles.
         * </p>
         *
         * @type {Boolean}
         * @default false
         */
        // this.debugColorizeTiles = Cesium.defaultValue(optionsParam.debugColorizeTiles, false);

        /**
         * This property is for debugging only; it is not optimized for production use.
         * <p>
         * When true, renders each tile's content as a wireframe.
         * </p>
         *
         * @type {Boolean}
         * @default false
         */
        // this.debugWireframe = Cesium.defaultValue(optionsParam.debugWireframe, false);

        /**
         * This property is for debugging only; it is not optimized for production use.
         * <p>
         * When true, renders the bounding volume for each visible tile.  The bounding volume is
         * white if the tile has a content bounding volume; otherwise, it is red.  Tiles that don't meet the
         * screen space error and are still refining to their descendants are yellow.
         * </p>
         *
         * @type {Boolean}
         * @default false
         */
        this.debugShowBoundingVolume = Cesium.defaultValue(optionsParam.debugShowBoundingVolume, true);

        /**
         * This property is for debugging only; it is not optimized for production use.
         * <p>
         * When true, renders the bounding volume for each visible tile's content. The bounding volume is
         * blue if the tile has a content bounding volume; otherwise it is red.
         * </p>
         *
         * @type {Boolean}
         * @default false
         */
        // this.debugShowContentBoundingVolume = Cesium.defaultValue(optionsParam.debugShowContentBoundingVolume, false);

        /**
         * This property is for debugging only; it is not optimized for production use.
         * <p>
         * When true, renders the viewer request volume for each tile.
         * </p>
         *
         * @type {Boolean}
         * @default false
         */
        // this.debugShowViewerRequestVolume = Cesium.defaultValue(optionsParam.debugShowViewerRequestVolume, false);

        // this._tileDebugLabels = undefined;
        // this.debugPickedTileLabelOnly = false;
        // this.debugPickedTile = undefined;
        // this.debugPickPosition = undefined;

        /**
         * This property is for debugging only; it is not optimized for production use.
         * <p>
         * When true, draws labels to indicate the geometric error of each tile.
         * </p>
         *
         * @type {Boolean}
         * @default false
         */
        // this.debugShowGeometricError = Cesium.defaultValue(optionsParam.debugShowGeometricError, false);

        /**
         * This property is for debugging only; it is not optimized for production use.
         * <p>
         * When true, draws labels to indicate the number of commands, points, triangles and features of each tile.
         * </p>
         *
         * @type {Boolean}
         * @default false
         */
        // this.debugShowRenderingStatistics = Cesium.defaultValue(optionsParam.debugShowRenderingStatistics, false);

        /**
         * This property is for debugging only; it is not optimized for production use.
         * <p>
         * When true, draws labels to indicate the geometry and texture memory usage of each tile.
         * </p>
         *
         * @type {Boolean}
         * @default false
         */
        // this.debugShowMemoryUsage = Cesium.defaultValue(optionsParam.debugShowMemoryUsage, false);

        /**
         * This property is for debugging only; it is not optimized for production use.
         * <p>
         * When true, draws labels to indicate the url of each tile.
         * </p>
         *
         * @type {Boolean}
         * @default false
         */
        // this.debugShowUrl = Cesium.defaultValue(optionsParam.debugShowUrl, false);
        this._credits = undefined;
        // hys 添加判断是否为m3d服务图层
        this._isM3dDataServer = -1;

        this.loadConfig(optionsParam.url);

        if (!Cesium.defined(Object.extend)) {
            Object.extend = function (destination, source) {
                for (var property in source) {
                    if (source.hasOwnProperty(property)) { //hys 注意这里要进行检验
                        var temObj = source[property];
                        if (temObj instanceof Array) {
                            if (destination[property] !== undefined) {
                                destination[property] = Object.extend(destination[property], temObj);
                            } else {
                                destination[property] = Object.extend([], temObj);
                            }
                        } else if (temObj instanceof Object) {
                            if (destination[property] !== undefined) {
                                destination[property] = Object.extend(destination[property], temObj);
                            } else {
                                destination[property] = Object.extend({}, temObj);
                            }
                        } else {
                            destination[property] = source[property];
                        }
                    }
                }
                return destination;
            }
        }
    }

    loadConfig(url) {
        const that = this;
        let tilesetResource;
        Cesium.when(url)
            .then((urlParam) => {
                // let basePath;
                const resource = Cesium.Resource.createIfNeeded(urlParam);
                that._isM3dDataServer = urlParam.lastIndexOf('/cache/');

                // ion resources have a credits property we can use for additional attribution.
                that._credits = resource.credits;

                tilesetResource = resource;
                if (that._isIGServer) {
                    that._basePath = tilesetResource.url;
                    // 添加对单独m3d服务的解析
                    if (that._isM3dDataServer > -1) {
                        tilesetResource.url += '?dataName=M3dTopInfoData&webGL=true';
                    } else {
                        tilesetResource.url += '&datatype=10&dataName=M3dTopInfoData&webGL=true&compress=false';
                    }
                } else if (resource.extension === 'json' || resource.extension === 'mcj') {
                    that._basePath = resource.getBaseUri(true);
                } else if (resource.isDataUri) {
                    that._basePath = '';
                } else {
                    resource.appendForwardSlash();
                    tilesetResource = resource.getDerivedResource({
                        url: 'tileset.json'
                    });
                    that._basePath = resource.url;
                }
                that._url = resource.url;
                that._tilesetUrl = tilesetResource.url;

                // We don't know the distance of the tileset until tileset.json is loaded, so use the default distance for now
                return MapGISM3DSet.loadJson(tilesetResource);
            })
            .then((tilesetJson) => {
                tilesetResource.url = tilesetResource.url.replace('M3dTopInfoData', '{dataName}');
                // that._geoBox = tilesetJson.geoBox;
                that._transform = Cesium.Transforms.eastNorthUpToFixedFrame(Cesium.Cartesian3.fromDegrees(tilesetJson.position.x, tilesetJson.position.y, 0.0));

                //准备好json上的信息:=================
                Object.extend(tilesetJson, {
                    boundingVolume: {
                        geoBox: tilesetJson.geoBox
                    },
                    refine: 'REPLACE'
                });
                tilesetJson.transform = that._transform;
                //========================================
                that._root = that.loadM3DDataSet(tilesetResource, tilesetJson);

                that._fieldInfo = tilesetJson.fieldInfo;
                // hys
                that._name = Cesium.defined(tilesetJson.layerName) ? tilesetJson.layerName : '';
                that._guid = Cesium.defined(tilesetJson.guid) ? tilesetJson.guid : '';
                const gltfUpAxis = Cesium.defined(tilesetJson.gltfUpAxis)
                    ? Cesium.Axis.fromName(tilesetJson.gltfUpAxis)
                    : Cesium.Axis.Y;
                const { asset } = tilesetJson;
                that._asset = asset;
                that._properties = tilesetJson.properties;
                that._geometricError = tilesetJson.geometricError;
                that._extensionsUsed = tilesetJson.extensionsUsed;
                that._gltfUpAxis = gltfUpAxis;
                that._extras = tilesetJson.extras;

                const { extras } = asset;
                if (Cesium.defined(extras) && Cesium.defined(extras.cesium) && Cesium.defined(extras.cesium.credits)) {
                    const extraCredits = extras.cesium.credits;
                    let credits = that._credits;
                    if (!Cesium.defined(credits)) {
                        credits = [];
                        that._credits = credits;
                    }
                    for (let i = 0; i < extraCredits.length; i += 1) {
                        const credit = extraCredits[i];
                        credits.push(new Cesium.Credit(credit.html, credit.showOnScreen));
                    }
                }

                // Save the original, untransformed bounding volume position so we can apply
                // the tile transform and model matrix at run time
                const boundingVolume = that._root.createBoundingVolume(
                    tilesetJson.boundingVolume,
                    Cesium.Matrix4.IDENTITY
                );
                // const clippingPlanesOrigin = boundingVolume.boundingSphere.center;
                // If this origin is above the surface of the earth
                // we want to apply an ENU orientation as our best guess of orientation.
                // Otherwise, we assume it gets its position/orientation completely from the
                // root tile transform and the tileset's model matrix
                // const originCartographic = that._ellipsoid.cartesianToCartographic(clippingPlanesOrigin);
                // if (
                //     Cesium.defined(originCartographic) &&
                //     originCartographic.height > Cesium.ApproximateTerrainHeights._defaultMinTerrainHeight
                // ) {
                //     that._initialClippingPlanesOriginMatrix = Cesium.Transforms.eastNorthUpToFixedFrame(
                //         clippingPlanesOrigin
                //     );
                // }
                // that._clippingPlanesOriginMatrix = Cesium.Matrix4.clone(that._initialClippingPlanesOriginMatrix);
                that._readyPromise.resolve(this);
            })
        .otherwise((error) => {
            that._readyPromise.reject(error);
        });
    }

    /**
     * Gets the tileset's asset object property, which contains metadata about the tileset.
     * <p>
     * See the {@link https://github.com/AnalyticalGraphicsInc/3d-tiles/blob/master/schema/asset.schema.json|asset schema}
     * in the 3D Tiles spec for the full set of properties.
     * </p>
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {Object}
     * @readonly
     *
     * @exception {DeveloperError} The tileset is not loaded.  Use MapGISM3DSet.readyPromise or wait for MapGISM3DSet.ready to be true.
     */
    get asset() {
        // >>includeStart('debug', pragmas.debug);
        if (!this.ready) {
            throw new Cesium.DeveloperError(
                'The tileset is not loaded.  Use MapGISM3DSet.readyPromise or wait for MapGISM3DSet.ready to be true.'
            );
        }
        // >>includeEnd('debug');

        return this._asset;
    }

    /**
     * The {@link ClippingPlaneCollection} used to selectively disable rendering the tileset.
     *
     * @type {ClippingPlaneCollection}
     */
    get clippingPlanes() {
        return this._clippingPlanes;
    }

    set clippingPlanes(value) {
        Cesium.ClippingPlaneCollection.setOwner(value, this, '_clippingPlanes');
    }

    /**
     * Gets the tileset's properties dictionary object, which contains metadata about per-feature properties.
     * <p>
     * See the {@link https://github.com/AnalyticalGraphicsInc/3d-tiles/blob/master/schema/properties.schema.json|properties schema}
     * in the 3D Tiles spec for the full set of properties.
     * </p>
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {Object}
     * @readonly
     *
     * @exception {DeveloperError} The tileset is not loaded.  Use MapGISM3DSet.readyPromise or wait for MapGISM3DSet.ready to be true.
     *
     * @example
     * console.log('Maximum building height: ' + tileset.properties.height.maximum);
     * console.log('Minimum building height: ' + tileset.properties.height.minimum);
     *
     * @see Cesium3DTileFeature#getProperty
     * @see Cesium3DTileFeature#setProperty
     */
    get properties() {
        // >>includeStart('debug', pragmas.debug);
        if (!this.ready) {
            throw new Cesium.DeveloperError(
                'The tileset is not loaded.  Use MapGISM3DSet.readyPromise or wait for MapGISM3DSet.ready to be true.'
            );
        }
        // >>includeEnd('debug');

        return this._properties;
    }

    /**
     * When <code>true</code>, the tileset's root tile is loaded and the tileset is ready to render.
     * This is set to <code>true</code> right before {@link MapGISM3DSet#readyPromise} is resolved.
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {Boolean}
     * @readonly
     *
     * @default false
     */
    get ready() {
        return Cesium.defined(this._root);
    }

    /**
     * Gets the promise that will be resolved when the tileset's root tile is loaded and the tileset is ready to render.
     * <p>
     * This promise is resolved at the end of the frame before the first frame the tileset is rendered in.
     * </p>
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {Promise.<MapGISM3DSet>}
     * @readonly
     *
     * @example
     * tileset.readyPromise.then(function(tileset) {
     *     // tile.properties is not defined until readyPromise resolves.
     *     let properties = tileset.properties;
     *     if (Cesium.defined(properties)) {
     *         for (let name in properties) {
     *             console.log(properties[name]);
     *         }
     *     }
     * });
     */
    get readyPromise() {
        return this._readyPromise.promise;
    }

    /**
     * When <code>true</code>, all tiles that meet the screen space error this frame are loaded. The tileset is
     * completely loaded for this view.
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {Boolean}
     * @readonly
     *
     * @default false
     *
     * @see MapGISM3DSet#allTilesLoaded
     */
    get tilesLoaded() {
        return this._tilesLoaded;
    }

    /**
     * The url to a tileset.json file or to a directory containing a tileset.json file.
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {String}
     * @readonly
     */
    get url() {
        return this._url;
    }

    /**
     * The base path that non-absolute paths in tileset.json are relative to.
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {String}
     * @readonly
     * @deprecated
     */
    get basePath() {
        Cesium.deprecationWarning(
            'MapGISM3DSet.basePath',
            'MapGISM3DSet.basePath has been deprecated. All tiles are relative to the url of the tileset.json that contains them. Use the url property instead.'
        );
        return this._basePath;
    }

    /**
     * The style, defined using the
     * {@link https://github.com/AnalyticalGraphicsInc/3d-tiles/tree/master/Styling|3D Tiles Styling language},
     * applied to each feature in the tileset.
     * <p>
     * Assign <code>undefined</code> to remove the style, which will restore the visual
     * appearance of the tileset to its default when no style was applied.
     * </p>
     * <p>
     * The style is applied to a tile before the {@link MapGISM3DSet#tileVisible}
     * event is raised, so code in <code>tileVisible</code> can manually set a feature's
     * properties (e.g. color and show) after the style is applied. When
     * a new style is assigned any manually set properties are overwritten.
     * </p>
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {Cesium3DTileStyle}
     *
     * @default undefined
     *
     * @example
     * tileset.style = new Cesium.Cesium3DTileStyle({
     *    color : {
     *        conditions : [
     *            ['${Height} >= 100', 'color("purple", 0.5)'],
     *            ['${Height} >= 50', 'color("red")'],
     *            ['true', 'color("blue")']
     *        ]
     *    },
     *    show : '${Height} > 0',
     *    meta : {
     *        description : '"Building id ${id} has height ${Height}."'
     *    }
     * });
     *
     * @see {@link https://github.com/AnalyticalGraphicsInc/3d-tiles/tree/master/Styling|3D Tiles Styling language}
     */
    get style() {
        return this._styleEngine.style;
    }

    set style(value) {
        this._styleEngine.style = value;
    }

    /**
     * The maximum screen space error used to drive level of detail refinement.  This value helps determine when a tile
     * refines to its descendants, and therefore plays a major role in balancing performance with visual quality.
     * <p>
     * A tile's screen space error is roughly equivalent to the number of pixels wide that would be drawn if a sphere with a
     * radius equal to the tile's <b>geometric error</b> were rendered at the tile's position. If this value exceeds
     * <code>maximumScreenSpaceError</code> the tile refines to its descendants.
     * </p>
     * <p>
     * Depending on the tileset, <code>maximumScreenSpaceError</code> may need to be tweaked to achieve the right balance.
     * Higher values provide better performance but lower visual quality.
     * </p>
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {Number}
     * @default 16
     *
     * @exception {DeveloperError} <code>maximumScreenSpaceError</code> must be greater than or equal to zero.
     */
    get maximumScreenSpaceError() {
        return this._maximumScreenSpaceError;
    }

    set maximumScreenSpaceError(value) {
        // >>includeStart('debug', pragmas.debug);
        Cesium.Check.typeOf.number.greaterThanOrEquals('maximumScreenSpaceError', value, 0);
        // >>includeEnd('debug');

        this._maximumScreenSpaceError = value;
    }

    /**
     * The maximum amount of GPU memory (in MB) that may be used to cache tiles. This value is estimated from
     * geometry, textures, and batch table textures of loaded tiles. For point clouds, this value also
     * includes per-point metadata.
     * <p>
     * Tiles not in view are unloaded to enforce this.
     * </p>
     * <p>
     * If decreasing this value results in unloading tiles, the tiles are unloaded the next frame.
     * </p>
     * <p>
     * If tiles sized more than <code>maximumMemoryUsage</code> are needed
     * to meet the desired screen space error, determined by {@link MapGISM3DSet#maximumScreenSpaceError},
     * for the current view, then the memory usage of the tiles loaded will exceed
     * <code>maximumMemoryUsage</code>.  For example, if the maximum is 256 MB, but
     * 300 MB of tiles are needed to meet the screen space error, then 300 MB of tiles may be loaded.  When
     * these tiles go out of view, they will be unloaded.
     * </p>
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {Number}
     * @default 512
     *
     * @exception {DeveloperError} <code>maximumMemoryUsage</code> must be greater than or equal to zero.
     * @see MapGISM3DSet#totalMemoryUsageInBytes
     */
    get maximumMemoryUsage() {
        return this._maximumMemoryUsage;
    }

    set maximumMemoryUsage(value) {
        // >>includeStart('debug', pragmas.debug);
        Cesium.Check.typeOf.number.greaterThanOrEquals('value', value, 0);
        // >>includeEnd('debug');

        this._maximumMemoryUsage = value;
    }

    /**
     * The root tile.
     *
     * @memberOf MapGISM3DSet.prototype
     *
     * @type {Cesium3DTile}
     * @readonly
     *
     * @exception {DeveloperError} The tileset is not loaded.  Use Cesium3DTileset.readyPromise or wait for Cesium3DTileset.ready to be true.
     */
    get root() {
        // >>includeStart('debug', pragmas.debug);
        if (!this.ready) {
            throw new Cesium.DeveloperError(
                'The tileset is not loaded.  Use Cesium3DTileset.readyPromise or wait for Cesium3DTileset.ready to be true.'
            );
        }
        // >>includeEnd('debug');

        return this._root;
    }

    /**
     * The tileset's bounding sphere.
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {BoundingSphere}
     * @readonly
     *
     * @exception {DeveloperError} The tileset is not loaded.  Use MapGISM3DSet.readyPromise or wait for MapGISM3DSet.ready to be true.
     *
     * @example
     * let tileset = viewer.scene.primitives.add(new Cesium.MapGISM3DSet({
     *     url : 'http://localhost:8002/tilesets/Seattle'
     * }));
     *
     * tileset.readyPromise.then(function(tileset) {
     *     // Set the camera to view the newly added tileset
     *     viewer.camera.viewBoundingSphere(tileset.boundingSphere, new Cesium.HeadingPitchRange(0, -0.5, 0));
     * });
     */
    get boundingSphere() {
        // >>includeStart('debug', pragmas.debug);
        if (!this.ready) {
            throw new Cesium.DeveloperError(
                'The tileset is not loaded.  Use MapGISM3DSet.readyPromise or wait for MapGISM3DSet.ready to be true.'
            );
        }
        // >>includeEnd('debug');

        this._root.updateTransform(this._modelMatrix);
        return this._root.boundingSphere;
    }

    /**
     * A 4x4 transformation matrix that transforms the entire tileset.
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {Matrix4}
     * @default Matrix4.IDENTITY
     *
     * @example
     * // Adjust a tileset's height from the globe's surface.
     * let heightOffset = 20.0;
     * let boundingSphere = tileset.boundingSphere;
     * let cartographic = Cesium.Cartographic.fromCartesian(boundingSphere.center);
     * let surface = Cesium.Cartesian3.fromRadians(cartographic.longitude, cartographic.latitude, 0.0);
     * let offset = Cesium.Cartesian3.fromRadians(cartographic.longitude, cartographic.latitude, heightOffset);
     * let translation = Cesium.Cartesian3.subtract(offset, surface, new Cesium.Cartesian3());
     * tileset.modelMatrix = Cesium.Matrix4.fromTranslation(translation);
     */
    get modelMatrix() {
        return this._modelMatrix;
    }

    set modelMatrix(value) {
        this._modelMatrix = Cesium.Matrix4.clone(value, this._modelMatrix);
    }

    /**
     * Returns the time, in milliseconds, since the tileset was loaded and first updated.
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {Number}
     * @readonly
     */
    get timeSinceLoad() {
        return this._timeSinceLoad;
    }

    /**
     * The total amount of GPU memory in bytes used by the tileset. This value is estimated from
     * geometry, texture, and batch table textures of loaded tiles. For point clouds, this value also
     * includes per-point metadata.
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {Number}
     * @readonly
     *
     * @see MapGISM3DSet#maximumMemoryUsage
     */
    get totalMemoryUsageInBytes() {
        const statistics = this._statistics;
        return statistics.texturesByteLength + statistics.geometryByteLength + statistics.batchTableByteLength;
    }

    /**
     * @private
     */
    get clippingPlanesOriginMatrix() {
        if (!Cesium.defined(this._clippingPlanesOriginMatrix)) {
            return Cesium.Matrix4.IDENTITY;
        }

        if (this._clippingPlanesOriginMatrixDirty) {
            Cesium.Matrix4.multiply(
                this.root.computedTransform,
                this._initialClippingPlanesOriginMatrix,
                this._clippingPlanesOriginMatrix
            );
            this._clippingPlanesOriginMatrixDirty = false;
        }

        return this._clippingPlanesOriginMatrix;
    }

    /**
     * @private
     */
    get styleEngine() {
        return this._styleEngine;
    }

    /**
     * @private
     */
    get statistics() {
        return this._statistics;
    }

    /**
     * Determines whether terrain, 3D Tiles or both will be classified by this tileset.
     * <p>
     * This option is only applied to tilesets containing batched 3D models, geometry data, or vector data. Even when undefined, vector data and geometry data
     * must render as classifications and will default to rendering on both terrain and other 3D Tiles tilesets.
     * </p>
     * <p>
     * When enabled for batched 3D model tilesets, there are a few requirements/limitations on the glTF:
     * <ul>
     *     <li>POSITION and _BATCHID semantics are required.</li>
     *     <li>All indices with the same batch id must occupy contiguous sections of the index buffer.</li>
     *     <li>All shaders and techniques are ignored. The generated shader simply multiplies the position by the model-view-projection matrix.</li>
     *     <li>The only supported extensions are CESIUM_RTC and WEB3D_quantized_attributes.</li>
     *     <li>Only one node is supported.</li>
     *     <li>Only one mesh per node is supported.</li>
     *     <li>Only one primitive per mesh is supported.</li>
     * </ul>
     * </p>
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {ClassificationType}
     * @default undefined
     *
     * @experimental This feature is using part of the 3D Tiles spec that is not final and is subject to change without Cesium's standard deprecation policy.
     * @readonly
     */
    get classificationType() {
        return this._classificationType;
    }

    /**
     * Gets an ellipsoid describing the shape of the globe.
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {Ellipsoid}
     * @readonly
     */
    get ellipsoid() {
        return this._ellipsoid;
    }

    /**
     * Optimization option. Used when {@link Cesium3DTileset#foveatedScreenSpaceError} is true to control the cone size that determines which tiles are deferred.
     * Tiles that are inside this cone are loaded immediately. Tiles outside the cone are potentially deferred based on how far outside the cone they are and {@link Cesium3DTileset#foveatedInterpolationCallback} and {@link Cesium3DTileset#foveatedMinimumScreenSpaceErrorRelaxation}.
     * Setting this to 0.0 means the cone will be the line formed by the camera position and its view direction. Setting this to 1.0 means the cone encompasses the entire field of view of the camera, essentially disabling the effect.
     *
     * @memberof MapGISM3DSet.prototype
     *
     * @type {Number}
     * @default 0.3
     */
    get foveatedConeSize() {
        return this._foveatedConeSize;
    }

    set foveatedConeSize(value) {
        // >>includeStart('debug', pragmas.debug);
        Cesium.Check.typeOf.number.greaterThanOrEquals('foveatedConeSize', value, 0.0);
        Cesium.Check.typeOf.number.lessThanOrEquals('foveatedConeSize', value, 1.0);
        // >>includeEnd('debug');

        this._foveatedConeSize = value;
    }

    /**
     * Optimization option. Used when {@link Cesium3DTileset#foveatedScreenSpaceError} is true to control the starting screen space error relaxation for tiles outside the foveated cone.
     * The screen space error will be raised starting with this value up to {@link Cesium3DTileset#maximumScreenSpaceError} based on the provided {@link Cesium3DTileset#foveatedInterpolationCallback}.
     *
     * @memberof
     *
     * @type {Number}
     * @default 0.0
     */
    get foveatedMinimumScreenSpaceErrorRelaxation() {
        return this._foveatedMinimumScreenSpaceErrorRelaxation;
    }

    set foveatedMinimumScreenSpaceErrorRelaxation(value) {
        // >>includeStart('debug', pragmas.debug);
        Cesium.Check.typeOf.number.greaterThanOrEquals('foveatedMinimumScreenSpaceErrorRelaxation', value, 0.0);
        Cesium.Check.typeOf.number.lessThanOrEquals(
            'foveatedMinimumScreenSpaceErrorRelaxation',
            value,
            this.maximumScreenSpaceError
        );
        // >>includeEnd('debug');

        this._foveatedMinimumScreenSpaceErrorRelaxation = value;
    }

    /**
     * Returns the <code>extras</code> property at the top-level of the tileset JSON, which contains application specific metadata.
     * Returns <code>undefined</code> if <code>extras</code> does not exist.
     *
     * @memberof
     *
     * @exception {DeveloperError} The tileset is not loaded.  Use Cesium3DTileset.readyPromise or wait for Cesium3DTileset.ready to be true.
     *
     * @type {*}
     * @readonly
     *
     * @see {@link https://github.com/AnalyticalGraphicsInc/3d-tiles/tree/master/specification#specifying-extensions-and-application-specific-extras|Extras in the 3D Tiles specification.}
     */
    get extras() {
        // >>includeStart('debug', pragmas.debug);
        if (!this.ready) {
            throw new Cesium.DeveloperError(
                'The tileset is not loaded.  Use Cesium3DTileset.readyPromise or wait for Cesium3DTileset.ready to be true.'
            );
        }
        // >>includeEnd('debug');

        return this._extras;
    }

    /**
     * 最小基准 小于这个容差的都设置为0 这个代表最后一个级别的error
     *  @memberof
     */
    get baseMinError() {
        return this._baseMinError;
    }

    set baseMinError(value) {
        this._baseMinError = value;
    }

    /**
     * 图层的 图层索引 可以用来区分组图层  一个组的图层 layerIndex是相同的 但是layerRenderIndex是不同的
     * @memberof
     */
    get layerIndex() {
        return this._layerIndex;
    }

    set layerIndex(value) {
        this._layerIndex = value;
    }

    /**
     * 图层的渲染索引：发布的图层的唯一索引。 图层自身的索引
     * @memberof
     */
    get layerRenderIndex() {
        return this._layerRenderIndex;
    }

    /**
     * 数据附带的 gdbp地址
     * @memberof
     */
    get gdbpUrl() {
        return this._gdbpUrl;
    }

    set gdbpUrl(value) {
        this._gdbpUrl = value;
    }

    /**
     * 图层名称
     * @memberof
     */
    get name() {
        return this._name;
    }

    set name(value) {
        this._name = value;
    }

    /**
     * 图层的guid
     * @memberof
     * @readonly
     */
    get guid() {
        return this._guid;
    }

    /**
     * 附加属性图层 //主要用于记录图层的单体化的 附加矢量图层。
     * @memberof
     */
    get extLayer() {
        return this._extLayer;
    }

    set extLayer(value) {
        this._extLayer = value;
    }

    get imageBasedLightingFactor() {
        return this._imageBasedLightingFactor;
    }

    set imageBasedLightingFactor(value) {
        // >>includeStart('debug', pragmas.debug);
        Cesium.Check.typeOf.object('imageBasedLightingFactor', value);
        Cesium.Check.typeOf.number.greaterThanOrEquals('imageBasedLightingFactor.x', value.x, 0.0);
        Cesium.Check.typeOf.number.lessThanOrEquals('imageBasedLightingFactor.x', value.x, 1.0);
        Cesium.Check.typeOf.number.greaterThanOrEquals('imageBasedLightingFactor.y', value.y, 0.0);
        Cesium.Check.typeOf.number.lessThanOrEquals('imageBasedLightingFactor.y', value.y, 1.0);
        // >>includeEnd('debug');
        Cesium.Cartesian2.clone(value, this._imageBasedLightingFactor);
    }

    /**
     * Provides a hook to override the method used to request the tileset json
     * useful when fetching tilesets from remote servers
     * @param {Resource|String} tilesetUrl The url of the json file to be fetched
     * @returns {Promise.<Object>} A promise that resolves with the fetched json data
     */
    static loadJson(tilesetUrl) {
        const resource = Cesium.Resource.createIfNeeded(tilesetUrl);
        return resource.fetchJson();
    }

    /**
     * Marks the tileset's {@link MapGISM3DSet#style} as dirty, which forces all
     * features to re-evaluate the style in the next frame each is visible.
     */
    makeStyleDirty() {
        this._styleEngine.makeDirty();
    }

    loadChildTileSet(m3dSetResource, indexJson, parentNode) {
        // 添加版本号参数
        const { asset } = indexJson;
        if (!Cesium.defined(m3dSetResource.queryParameters.v)) {
            const versionQuery = {
                v: Cesium.defaultValue(asset.tilesetVersion, '0.0')
            };
            this._basePath += `?v=${versionQuery.v}`;
            m3dSetResource.setQueryParameters(versionQuery);
        }

        const statistics = this._statistics;
        // Object.extend(indexJson, {
        //     boundingVolume: parentNode._header.boundingVolume
        // });
        var rootTile = new MapGISM3D(this, m3dSetResource, indexJson, parentNode);

        if (Cesium.defined(parentNode)) {
            parentNode.children.push(rootTile);
            rootTile._depth = parentNode._depth + 1;
        }
        const stack = [];
        stack.push(rootTile);
        // stack.push({
        //     header: indexJson.children, // 这里后面直接就是children
        //     tile3D: parentNode
        // });

        while (stack.length > 0) {
            const tile = stack.pop();
            // let tile3D = tile.tile3D;
            // var children = tile.header;
            const { children } = tile._header;
            statistics.numberOfTilesTotal += 1;
            this._allTilesAdditive = this._allTilesAdditive && tile.refine === Cesium.Cesium3DTileRefine.ADD;
            if (Cesium.defined(children)) {
                const { length } = children;
                for (let i = 0; i < length; i += 1) {
                    const childHeader = children[i];
                    if (childHeader.lodError < this._baseMinError) {
                        childHeader.lodError = 0;
                    }
                    // Object.extend(childHeader, {
                    //     refine: 'REPLACE'
                    // });

                    const childTile = new MapGISM3D(this, m3dSetResource, childHeader, tile);
                    rootTile.children.push(childTile);
                    rootTile.childrenNameList.push(childHeader.uri);
                    childTile._depth = parentNode._depth + 1;
                    // statistics.numberOfTilesTotal += 1;
                    stack.push(childTile);
                    // stack.push({
                    //     header: childHeader,
                    //     tile3D: childTile
                    // });
                }
            }

            if (this._cullWithChildrenBounds) {
                Cesium.Cesium3DTileOptimizations.checkChildrenWithinParent(parentNode);
            }
        }

        return parentNode;
    }

    /**
     * Loads the main tileset.json or a tileset.json referenced from a tile.
     *
     * @private
     */
    loadM3DDataSet(dataSetResource, dataSetJson, parentTile) {
        const { asset } = dataSetJson;
        // if (!Cesium.defined(asset)) {
        //     throw new Cesium.RuntimeError('Tileset must have an asset property.');
        // }
        // if (dataSetJson.version !== '0.0' && dataSetJson.version !== '1.0') {
        //     throw new Cesium.RuntimeError('版本信息不匹配');
        // }

        const statistics = this._statistics;

        // 后面加一个版本参数 用来标识数据的版本信息
        if (!Cesium.defined(dataSetResource.queryParameters.v)) {
            const versionQuery = {
                v: Cesium.defaultValue(asset.tilesetVersion, '0.0')
            };
            this._basePath += `?v=${versionQuery.v}`;
            dataSetResource.setQueryParameters(versionQuery);
        }
        //fgy新版数据请求rootNode========================
        // var url = dataSetResource.url;
        // url = url.substring(0,url.lastIndexOf('/')+1);
        // url += dataSetJson.uri;
        //  const resource = Cesium.Resource.createIfNeeded(url);
        //  var promise = MapGISM3DSet.loadJson(resource);
        //  promise.then((rootNodeJson)=>{
        //      var root = rootNodeJson;
        //     // 创建根节点
        //     Object.extend(root,{
        //         boundingVolume:{
        //             geoBox:this._geoBox
        //         },
        //         // boundingVolume: {
        //             // region: [
        //             //     1.9954349994659424,
        //             //     0.5323253273963928,
        //             //     1.9954394102096558,
        //             //     0.532336413860321,
        //             //     0.0,
        //             //     15.327899932861329
        //             // ]
        //             // regionBox: [
        //             //     -3.552713678800501e-14,
        //             //     -0.00000286102294921875,
        //             //     0.0,
        //             //     23.904619216918947,
        //             //     69.44810485839844,
        //             //     15.327899932861329
        //             // ]
        //         // },
        //         refine:'REPLACE'});

        // rootNodeJson.transform = this._transform;
        // const rootNodes = new MapGISM3D(this, dataSetResource, rootNodeJson, parentTile);

        // if (Cesium.defined(parentTile)) {
        //     parentTile.children.push(rootNodes);
        //     rootNodes._depth = parentTile._depth + 1;
        // }
        // const stack = [];
        // stack.push(rootNodes);

        // while (stack.length > 0) {
        //     const tile = stack.pop();
        //     statistics.numberOfTilesTotal += 1;
        //     this._allTilesAdditive = this._allTilesAdditive && tile.refine === Cesium.Cesium3DTileRefine.ADD;
        //     const { childrenNode } = tile._header;
        //     if (Cesium.defined(childrenNode)) {
        //         const { length } = childrenNode;
        //         for (let i = 0; i < length; i += 1) {
        //             const childHeader = childrenNode[i];
        //             // Object.extend(childHeader,{
        //             //     boundingVolume: {
        //             //         region: [
        //             //                 1.9954349994659424,
        //             //                 0.5323253273963928,
        //             //                 1.995436429977417,
        //             //                 0.532336413860321,
        //             //                 0.0,
        //             //                 15.327899932861329
        //             //             ]
        //             //         // regionBox: [
        //             //         //     -3.552713678800501e-14,
        //             //         //     -0.00000286102294921875,
        //             //         //     0.0,
        //             //         //     7.298200607299805,
        //             //         //     69.44810485839844, 
        //             //         //     15.327899932861329]
        //             //     }
        //             // });
        //             Object.extend(childHeader,{boundingVolume: {geoBox:childHeader.geoBox}});
        //             const childTile = new MapGISM3D(this, dataSetResource, childHeader, tile);
        //             tile.children.push(childTile);
        //             childTile._depth = tile._depth + 1;
        //             stack.push(childTile);
        //         }
        //     }

        //     if (this._cullWithChildrenBounds) {
        //         Cesium.Cesium3DTileOptimizations.checkChildrenWithinParent(tile);
        //     }
        // }

        //  this._root = rootNodes;
        //  var boundingVolume = this._root.createBoundingVolume(rootNodeJson.boundingVolume, Cesium.Matrix4.IDENTITY);
        //  var clippingPlanesOrigin = boundingVolume.boundingSphere.center;
        //  const originCartographic = this._ellipsoid.cartesianToCartographic(clippingPlanesOrigin);
        //         if (
        //             Cesium.defined(originCartographic) &&
        //             originCartographic.height > Cesium.ApproximateTerrainHeights._defaultMinTerrainHeight
        //         ) {
        //             this._initialClippingPlanesOriginMatrix = Cesium.Transforms.eastNorthUpToFixedFrame(
        //                 clippingPlanesOrigin
        //             );
        //         }
        //  this._clippingPlanesOriginMatrix = Cesium.Matrix4.clone(this._initialClippingPlanesOriginMatrix);
        //  this._readyPromise.resolve(this);
        // });
        //==============================================

        // A tileset JSON file referenced from a tile may exist in a different directory than the root tileset.
        // Get the basePath relative to the external tileset.
        // 创建根节点
        // const rootNode = new MapGISM3D(this, dataSetResource, dataSetJson.root, parentTile);


        // 创建根节点
        // Object.extend(dataSetJson, {
        //     boundingVolume: {
        //         geoBox: this._geoBox
        //     },
        //     refine: 'REPLACE'
        // });
        // dataSetJson.transform = this._transform;
        const rootNode = new MapGISM3D(this, dataSetResource, dataSetJson, parentTile);

        // If there is a parentTile, add the root of the currently loading tileset
        // to parentTile's children, and update its _depth.
        if (Cesium.defined(parentTile)) {
            parentTile.children.push(rootNode);
            rootNode._depth = parentTile._depth + 1;
        }

        // 加载子节点
        // this.loadChildNode(dataSetResource,dataSetJson.root,rootNode);
        const stack = [];
        stack.push(rootNode);

        while (stack.length > 0) {
            const tile = stack.pop();
            statistics.numberOfTilesTotal += 1;
            this._allTilesAdditive = this._allTilesAdditive && tile.refine === Cesium.Cesium3DTileRefine.ADD;
            const { children } = tile._header;
            if (Cesium.defined(children)) {
                const { length } = children;
                for (let i = 0; i < length; i += 1) {
                    const childHeader = children[i];
                    const childTile = new MapGISM3D(this, dataSetResource, childHeader, tile);
                    tile.children.push(childTile);
                    tile.childrenNameList.push(childHeader.uri);
                    childTile._depth = tile._depth + 1;
                    stack.push(childTile);
                }
            }

            if (this._cullWithChildrenBounds) {
                Cesium.Cesium3DTileOptimizations.checkChildrenWithinParent(tile);
            }
        }

        return rootNode;
    }

    /**
     * Perform any pass invariant tasks here. Called after the render pass.
     * @private
     */
    postPassesUpdate(frameState) {
        if (!this.ready) {
            return;
        }

        cancelOutOfViewRequests(this, frameState);
        raiseLoadProgressEvent(this, frameState);
        this._cache.unloadTiles(this, unloadTile);
    }

    /**
     * Perform any pass invariant tasks here. Called before any passes are executed.
     * @private
     */
    prePassesUpdate(frameState) {
        if (!this.ready) {
            return;
        }

        processTiles(this, frameState);

        // Update clipping planes
        const clippingPlanes = this._clippingPlanes;
        this._clippingPlanesOriginMatrixDirty = true;
        if (Cesium.defined(clippingPlanes) && clippingPlanes.enabled) {
            clippingPlanes.update(frameState);
        }

        if (!Cesium.defined(this._loadTimestamp)) {
            this._loadTimestamp = Cesium.JulianDate.clone(frameState.time);
        }
        this._timeSinceLoad = Math.max(
            Cesium.JulianDate.secondsDifference(frameState.time, this._loadTimestamp) * 1000,
            0.0
        );

        this._skipLevelOfDetail =
            this.skipLevelOfDetail &&
            !Cesium.defined(this._classificationType) &&
            !this._disableSkipLevelOfDetail &&
            !this._allTilesAdditive;

        if (this.dynamicScreenSpaceError) {
            updateDynamicScreenSpaceError(this, frameState);
        }

        if (frameState.newFrame) {
            this._cache.reset();
        }
    }

    /**
     * Unloads all tiles that weren't selected the previous frame.  This can be used to
     * explicitly manage the tile cache and reduce the total number of tiles loaded below
     * {@link MapGISM3DSet#maximumMemoryUsage}.
     * <p>
     * Tile unloads occur at the next frame to keep all the WebGL delete calls
     * within the render loop.
     * </p>
     */
    trimLoadedTiles() {
        this._cache.trim();
    }

    /**
     * @private
     */
    update(frameState) {
        this.updateForPass(frameState, frameState.tilesetPassState);
    }

    // hys
    addPlanishPolygon(polygon) {
        if (this._planishPolygons) {
            this._planishPolygons.push(polygon);
        }
        initPlanishPolygon(this);
    }

    setScale(scale) {
        if (!scale) {
            return;
        }
        this._root.transform = Cesium.Matrix4.setScale(this._root.transform, scale, this._root.transform);
    }

    setTranslation(translation) {
        if (!translation) {
            return;
        }
        // this._root.transform = Matrix4.setTranslation(this._root.transform,translation, this._root.transform);
        this.modelMatrix = Cesium.Matrix4.fromTranslation(translation);
    }

    setRotation(rotation) {
        if (!rotation) {
            // Matrix3
            return;
        }
        this._root.transform = Cesium.Matrix4.multiplyByMatrix3(this._root.transform, rotation, this._root.transform);
    }

    setTranslationRotationScale(translation, rotation, scale) {
        this.setTranslation(translation);
        this.setRotation(rotation);
        this.setScale(scale);
    }

    /**
     * Returns true if this object was destroyed; otherwise, false.
     * <br /><br />
     * If this object was destroyed, it should not be used; calling any function other than
     * <code>isDestroyed</code> will result in a {@link DeveloperError} exception.
     *
     * @returns {Boolean} <code>true</code> if this object was destroyed; otherwise, <code>false</code>.
     *
     * @see MapGISM3DSet#destroy
     */
    // eslint-disable-next-line class-methods-use-this
    isDestroyed() {
        return false;
    }

    updateForPass(frameStateParam, tilesetPassStateParam) {
        const frameState = frameStateParam;
        const tilesetPassState = tilesetPassStateParam;
        // >>includeStart('debug', pragmas.debug);
        Cesium.Check.typeOf.object('frameState', frameState);
        Cesium.Check.typeOf.object('tilesetPassState', tilesetPassState);
        // >>includeEnd('debug');

        const { pass } = tilesetPassState;
        if (
            (pass === Cesium.Cesium3DTilePass.PRELOAD && (!this.preloadWhenHidden || this.show)) ||
            (pass === Cesium.Cesium3DTilePass.PRELOAD_FLIGHT &&
                (!this.preloadFlightDestinations || (!this.show && !this.preloadWhenHidden))) ||
            (pass === Cesium.Cesium3DTilePass.REQUEST_RENDER_MODE_DEFER_CHECK &&
                !this.cullRequestsWhileMoving &&
                this.foveatedTimeDelay <= 0)
        ) {
            return;
        }

        const originalCommandList = frameState.commandList;
        const originalCamera = frameState.camera;
        const originalCullingVolume = frameState.cullingVolume;

        tilesetPassState.ready = false;

        const passOptions = Cesium.Cesium3DTilePass.getPassOptions(pass);
        const { ignoreCommands } = passOptions;

        const commandList = Cesium.defaultValue(tilesetPassState.commandList, originalCommandList);
        const commandStart = commandList.length;

        frameState.commandList = commandList;
        frameState.camera = Cesium.defaultValue(tilesetPassState.camera, originalCamera);
        frameState.cullingVolume = Cesium.defaultValue(tilesetPassState.cullingVolume, originalCullingVolume);

        const passStatistics = this._statisticsPerPass[pass];

        if (this.show || ignoreCommands) {
            this._pass = pass;
            tilesetPassState.ready = update(this, frameState, passStatistics, passOptions);
        }

        if (ignoreCommands) {
            commandList.length = commandStart;
        }

        frameState.commandList = originalCommandList;
        frameState.camera = originalCamera;
        frameState.cullingVolume = originalCullingVolume;
    }

    /**
     * <code>true</code> if the tileset JSON file lists the extension in extensionsUsed; otherwise, <code>false</code>.
     * @param {String} extensionName The name of the extension to check.
     *
     * @returns {Boolean} <code>true</code> if the tileset JSON file lists the extension in extensionsUsed; otherwise, <code>false</code>.
     */
    hasExtension(extensionName) {
        if (!Cesium.defined(this._extensionsUsed)) {
            return false;
        }

        return this._extensionsUsed.indexOf(extensionName) > -1;
    }

    /**
     * Destroys the WebGL resources held by this object.  Destroying an object allows for deterministic
     * release of WebGL resources, instead of relying on the garbage collector to destroy this object.
     * <br /><br />
     * Once an object is destroyed, it should not be used; calling any function other than
     * <code>isDestroyed</code> will result in a {@link DeveloperError} exception.  Therefore,
     * assign the return value (<code>undefined</code>) to the object as done in the example.
     *
     * @exception {DeveloperError} This object was destroyed, i.e., destroy() was called.
     *
     * @example
     * tileset = tileset && tileset.destroy();
     *
     * @see Cesium3DTileset#isDestroyed
     */
    destroy() {
        this._tileDebugLabels = this._tileDebugLabels && this._tileDebugLabels.destroy();
        this._clippingPlanes = this._clippingPlanes && this._clippingPlanes.destroy();

        // Traverse the tree and destroy all tiles
        if (Cesium.defined(this._root)) {
            const stack = scratchStack;
            stack.push(this._root);

            while (stack.length > 0) {
                const tile = stack.pop();
                tile.destroy();

                const { children } = tile;
                const { length } = children;
                for (let i = 0; i < length; i += 1) {
                    stack.push(children[i]);
                }
            }
        }

        this._root = undefined;
        return Cesium.destroyObject(this);
    }
}

CesiumZondy.M3D.MapGISM3DSet = MapGISM3DSet;
