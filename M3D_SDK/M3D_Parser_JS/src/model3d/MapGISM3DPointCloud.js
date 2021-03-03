import { CesiumZondy } from '../core/Base';
import getClippingFunction from '../shaders/getClippingFunction';
import getClipAndStyleCode from '../shaders/getClipAndStyleCode';

const DecodingState = {
    NEEDS_DECODE: 0,
    DECODING: 1,
    READY: 2,
    FAILED: 3
};

const sizeOfUint32 = Uint32Array.BYTES_PER_ELEMENT;

function initialize(pointCloudParam, options) {
    const pointCloud = pointCloudParam;
    // 初始化变量，后面需要做容错处理.
    const { view } = options.param;
    let { byteOffset } = options.param;
    const { uint8Array } = options.param;
    const { tileset } = options.param;
    const { resource } = options.param;
    const { parentNode } = options.param;
    const { arrayBuffer } = options.param;
    // let content = options.param.content;
    // let byteStart = options.param.byteStart;
    // let allLength = options.param.allLength;

    const indexJsonLength = view.getUint32(byteOffset, true);
    byteOffset += sizeOfUint32;

    const featureTableJsonByteLength = view.getUint32(byteOffset, true);
    if (featureTableJsonByteLength === 0) {
        throw new Cesium.RuntimeError('Feature table must have a byte length greater than zero');
    }
    byteOffset += sizeOfUint32;

    const featureTableBinaryByteLength = view.getUint32(byteOffset, true);
    byteOffset += sizeOfUint32;

    // let batchTableJsonByteLength = view.getUint32(byteOffset, true);
    // byteOffset += sizeOfUint32;
    const batchTableJsonByteLength = 0;
    const batchTableBinaryByteLength = view.getUint32(byteOffset, true);
    byteOffset += sizeOfUint32;

    // 临时将此处的点云索引取出，以待后用.
    if (indexJsonLength > 0) {
        const indexTableString = Cesium.getStringFromTypedArray(uint8Array, byteOffset, indexJsonLength);
        const indexJson = JSON.parse(indexTableString);
        // 加载子节点.
        tileset.loadChildTileSet(resource, indexJson, parentNode);
    }
    byteOffset += indexJsonLength;

    const featureTableString = Cesium.getStringFromTypedArray(uint8Array, byteOffset, featureTableJsonByteLength);
    const featureTableJson = JSON.parse(featureTableString);
    byteOffset += featureTableJsonByteLength;

    const featureTableBinary = new Uint8Array(arrayBuffer, byteOffset, featureTableBinaryByteLength);
    byteOffset += featureTableBinaryByteLength;

    // Get the batch table JSON and binary
    let batchTableJson;
    let batchTableBinary;
    if (batchTableJsonByteLength > 0) {
        // Has a batch table JSON
        const batchTableString = Cesium.getStringFromTypedArray(uint8Array, byteOffset, batchTableJsonByteLength);
        batchTableJson = JSON.parse(batchTableString);
        byteOffset += batchTableJsonByteLength;

        if (batchTableBinaryByteLength > 0) {
            // Has a batch table binary
            batchTableBinary = new Uint8Array(arrayBuffer, byteOffset, batchTableBinaryByteLength);
            byteOffset += batchTableBinaryByteLength;
        }
    }

    const featureTable = new Cesium.Cesium3DTileFeatureTable(featureTableJson, featureTableBinary);

    const pointsLength = featureTable.getGlobalProperty('POINTS_LENGTH');
    featureTable.featuresLength = pointsLength;

    if (!Cesium.defined(pointsLength)) {
        throw new Cesium.RuntimeError('Feature table global property: POINTS_LENGTH must be defined');
    }

    const rtcCenter = featureTable.getGlobalProperty('RTC_CENTER', Cesium.ComponentDatatype.FLOAT, 3);
    if (Cesium.defined(rtcCenter)) {
        pointCloud._rtcCenter = Cesium.Cartesian3.unpack(rtcCenter);
    }

    let positions;
    let colors;
    let normals;
    let batchIds;

    let hasPositions = false;
    let hasColors = false;
    let hasNormals = false;
    let hasBatchIds = false;

    let isQuantized = false;
    let isTranslucent = false;
    let isRGB565 = false;
    let isOctEncoded16P = false;

    let dracoBuffer;
    let dracoFeatureTableProperties;
    let dracoBatchTableProperties;

    const featureTableDraco = Cesium.defined(featureTableJson.extensions)
        ? featureTableJson.extensions['3DTILES_draco_point_compression']
        : undefined;
    const batchTableDraco =
        Cesium.defined(batchTableJson) && Cesium.defined(batchTableJson.extensions)
            ? batchTableJson.extensions['3DTILES_draco_point_compression']
            : undefined;

    if (Cesium.defined(batchTableDraco)) {
        dracoBatchTableProperties = batchTableDraco.properties;
    }

    if (Cesium.defined(featureTableDraco)) {
        dracoFeatureTableProperties = featureTableDraco.properties;
        const dracoByteOffset = featureTableDraco.byteOffset;
        const dracoByteLength = featureTableDraco.byteLength;
        if (
            !Cesium.defined(dracoFeatureTableProperties) ||
            !Cesium.defined(dracoByteOffset) ||
            !Cesium.defined(dracoByteLength)
        ) {
            throw new Cesium.RuntimeError('Draco properties, byteOffset, and byteLength must be defined');
        }

        dracoBuffer = Cesium.arraySlice(featureTableBinary, dracoByteOffset, dracoByteOffset + dracoByteLength);
        hasPositions = Cesium.defined(dracoFeatureTableProperties.POSITION);
        hasColors = Cesium.defined(dracoFeatureTableProperties.RGB) || Cesium.defined(dracoFeatureTableProperties.RGBA);
        hasNormals = Cesium.defined(dracoFeatureTableProperties.NORMAL);
        hasBatchIds = Cesium.defined(dracoFeatureTableProperties.BATCH_ID);
        isTranslucent = Cesium.defined(dracoFeatureTableProperties.RGBA);
        pointCloud._decodingState = DecodingState.NEEDS_DECODE;
    }

    let draco;
    if (Cesium.defined(dracoBuffer)) {
        draco = {
            buffer: dracoBuffer,
            featureTableProperties: dracoFeatureTableProperties,
            batchTableProperties: dracoBatchTableProperties,
            properties: Cesium.combine(dracoFeatureTableProperties, dracoBatchTableProperties),
            dequantizeInShader: pointCloud._dequantizeInShader
        };
    }

    if (!hasPositions) {
        if (Cesium.defined(featureTableJson.POSITION)) {
            positions = featureTable.getPropertyArray('POSITION', Cesium.ComponentDatatype.FLOAT, 3);
            hasPositions = true;
        } else if (Cesium.defined(featureTableJson.POSITION_QUANTIZED)) {
            positions = featureTable.getPropertyArray('POSITION_QUANTIZED', Cesium.ComponentDatatype.UNSIGNED_SHORT, 3);
            isQuantized = true;
            hasPositions = true;

            const quantizedVolumeScale = featureTable.getGlobalProperty(
                'QUANTIZED_VOLUME_SCALE',
                Cesium.ComponentDatatype.FLOAT,
                3
            );
            if (!Cesium.defined(quantizedVolumeScale)) {
                throw new Cesium.RuntimeError(
                    'Global property: QUANTIZED_VOLUME_SCALE must be defined for quantized positions.'
                );
            }
            pointCloud._quantizedVolumeScale = Cesium.Cartesian3.unpack(quantizedVolumeScale);
            pointCloud._quantizedRange = 2 ** 16 - 1;

            const quantizedVolumeOffset = featureTable.getGlobalProperty(
                'QUANTIZED_VOLUME_OFFSET',
                Cesium.ComponentDatatype.FLOAT,
                3
            );
            if (!Cesium.defined(quantizedVolumeOffset)) {
                throw new Cesium.RuntimeError(
                    'Global property: QUANTIZED_VOLUME_OFFSET must be defined for quantized positions.'
                );
            }
            pointCloud._quantizedVolumeOffset = Cesium.Cartesian3.unpack(quantizedVolumeOffset);
        }
    }

    if (!hasColors) {
        if (Cesium.defined(featureTableJson.RGBA)) {
            colors = featureTable.getPropertyArray('RGBA', Cesium.ComponentDatatype.UNSIGNED_BYTE, 4);
            isTranslucent = true;
            hasColors = true;
        } else if (Cesium.defined(featureTableJson.RGB)) {
            colors = featureTable.getPropertyArray('RGB', Cesium.ComponentDatatype.UNSIGNED_BYTE, 3);
            hasColors = true;
        } else if (Cesium.defined(featureTableJson.RGB565)) {
            colors = featureTable.getPropertyArray('RGB565', Cesium.ComponentDatatype.UNSIGNED_SHORT, 1);
            isRGB565 = true;
            hasColors = true;
        }
    }

    if (!hasNormals) {
        if (Cesium.defined(featureTableJson.NORMAL)) {
            normals = featureTable.getPropertyArray('NORMAL', Cesium.ComponentDatatype.FLOAT, 3);
            hasNormals = true;
        } else if (Cesium.defined(featureTableJson.NORMAL_OCT16P)) {
            normals = featureTable.getPropertyArray('NORMAL_OCT16P', Cesium.ComponentDatatype.UNSIGNED_BYTE, 2);
            isOctEncoded16P = true;
            hasNormals = true;
        }
    }

    if (!hasBatchIds) {
        if (Cesium.defined(featureTableJson.BATCH_ID)) {
            batchIds = featureTable.getPropertyArray('BATCH_ID', Cesium.ComponentDatatype.UNSIGNED_SHORT, 1);
            hasBatchIds = true;
        }
    }

    if (!hasPositions) {
        throw new Cesium.RuntimeError('Either POSITION or POSITION_QUANTIZED must be defined.');
    }

    if (Cesium.defined(featureTableJson.CONSTANT_RGBA)) {
        const constantRGBA = featureTable.getGlobalProperty('CONSTANT_RGBA', Cesium.ComponentDatatype.UNSIGNED_BYTE, 4);
        pointCloud._constantColor = Cesium.Color.fromBytes(
            constantRGBA[0],
            constantRGBA[1],
            constantRGBA[2],
            constantRGBA[3],
            pointCloud._constantColor
        );
    }

    if (hasBatchIds) {
        const batchLength = featureTable.getGlobalProperty('BATCH_LENGTH');
        if (!Cesium.defined(batchLength)) {
            throw new Cesium.RuntimeError('Global property: BATCH_LENGTH must be defined when BATCH_ID is defined.');
        }

        if (Cesium.defined(batchTableBinary)) {
            // Copy the batchTableBinary section and let the underlying ArrayBuffer be freed
            batchTableBinary = new Uint8Array(batchTableBinary);
        }

        if (Cesium.defined(pointCloud._batchTableLoaded)) {
            pointCloud._batchTableLoaded(batchLength, batchTableJson, batchTableBinary);
        }
    }

    // If points are not batched and there are per-point properties, use these properties for styling purposes
    let styleableProperties;
    if (!hasBatchIds && Cesium.defined(batchTableBinary)) {
        styleableProperties = Cesium.Cesium3DTileBatchTable.getBinaryProperties(
            pointsLength,
            batchTableJson,
            batchTableBinary
        );
    }

    pointCloud._parsedContent = {
        positions,
        colors,
        normals,
        batchIds,
        styleableProperties,
        draco
    };
    pointCloud._pointsLength = pointsLength;
    pointCloud._isQuantized = isQuantized;
    pointCloud._isOctEncoded16P = isOctEncoded16P;
    pointCloud._isRGB565 = isRGB565;
    pointCloud._isTranslucent = isTranslucent;
    pointCloud._hasColors = hasColors;
    pointCloud._hasNormals = hasNormals;
    pointCloud._hasBatchIds = hasBatchIds;
}

const scratchMin = new Cesium.Cartesian3();
const scratchMax = new Cesium.Cartesian3();
const scratchPosition = new Cesium.Cartesian3();

function getRandomValues(samplesLength) {
    let randomValues;
    // Use same random values across all runs
    if (!Cesium.defined(randomValues)) {
        Cesium.Math.setRandomNumberSeed(0);
        randomValues = new Array(samplesLength);
        for (let i = 0; i < samplesLength; i += 1) {
            randomValues[i] = Cesium.Math.nextRandomNumber();
        }
    }
    return randomValues;
}

function computeApproximateBoundingSphereFromPositions(positions) {
    const maximumSamplesLength = 20;
    const pointsLength = positions.length / 3;
    const samplesLength = Math.min(pointsLength, maximumSamplesLength);
    const randomValues = getRandomValues(maximumSamplesLength);
    const maxValue = Number.MAX_VALUE;
    const minValue = -Number.MAX_VALUE;
    const min = Cesium.Cartesian3.fromElements(maxValue, maxValue, maxValue, scratchMin);
    const max = Cesium.Cartesian3.fromElements(minValue, minValue, minValue, scratchMax);
    for (let i = 0; i < samplesLength; i += 1) {
        const index = Math.floor(randomValues[i] * pointsLength);
        const position = Cesium.Cartesian3.unpack(positions, index * 3, scratchPosition);
        Cesium.Cartesian3.minimumByComponent(min, position, min);
        Cesium.Cartesian3.maximumByComponent(max, position, max);
    }

    const boundingSphere = Cesium.BoundingSphere.fromCornerPoints(min, max);
    boundingSphere.radius += Cesium.Math.EPSILON2; // To avoid radius of zero
    return boundingSphere;
}

function prepareVertexAttribute(typedArray, name) {
    // WebGL does not support UNSIGNED_INT, INT, or DOUBLE vertex attributes. Convert these to FLOAT.
    const componentDatatype = Cesium.ComponentDatatype.fromTypedArray(typedArray);
    if (
        componentDatatype === Cesium.ComponentDatatype.INT ||
        componentDatatype === Cesium.ComponentDatatype.UNSIGNED_INT ||
        componentDatatype === Cesium.ComponentDatatype.DOUBLE
    ) {
        Cesium.oneTimeWarning(
            'Cast pnts property to floats',
            `Point cloud property "${name}" will be casted to a float array because INT, UNSIGNED_INT, and DOUBLE are not valid WebGL vertex attribute types. Some precision may be lost.`
        );
        return new Float32Array(typedArray);
    }
    return typedArray;
}

const scratchPointSizeAndTimeAndGeometricErrorAndDepthMultiplier = new Cesium.Cartesian4();
const scratchQuantizedVolumeScaleAndOctEncodedRange = new Cesium.Cartesian4();
const scratchColor = new Cesium.Color();

const positionLocation = 0;
const colorLocation = 1;
const normalLocation = 2;
const batchIdLocation = 3;
const numberOfAttributes = 4;

const scratchClippingPlaneMatrix = new Cesium.Matrix4();

function createResources(pointCloudParam, frameState) {
    const pointCloud = pointCloudParam;
    const { context } = frameState;
    const parsedContent = pointCloud._parsedContent;
    const pointsLength = pointCloud._pointsLength;
    const { positions } = parsedContent;
    const { colors } = parsedContent;
    const { normals } = parsedContent;
    let { batchIds } = parsedContent;
    const { styleableProperties } = parsedContent;
    const hasStyleableProperties = Cesium.defined(styleableProperties);
    const isQuantized = pointCloud._isQuantized;
    const isQuantizedDraco = pointCloud._isQuantizedDraco;
    const isOctEncoded16P = pointCloud._isOctEncoded16P;
    const isOctEncodedDraco = pointCloud._isOctEncodedDraco;
    const quantizedRange = pointCloud._quantizedRange;
    const octEncodedRange = pointCloud._octEncodedRange;
    const isRGB565 = pointCloud._isRGB565;
    const isTranslucent = pointCloud._isTranslucent;
    const hasColors = pointCloud._hasColors;
    const hasNormals = pointCloud._hasNormals;
    const hasBatchIds = pointCloud._hasBatchIds;

    let componentsPerAttribute;
    let componentDatatype;

    const styleableVertexAttributes = [];
    const styleableShaderAttributes = {};
    pointCloud._styleableShaderAttributes = styleableShaderAttributes;

    if (hasStyleableProperties) {
        let attributeLocation = numberOfAttributes;

        // for (const name in styleableProperties) {
        Object.getOwnPropertyNames(styleableProperties).forEach((name) => {
            // if (styleableProperties.hasOwnProperty(name)) {
            if (Object.prototype.hasOwnProperty.call(styleableProperties, name)) {
                const property = styleableProperties[name];
                const typedArray = prepareVertexAttribute(property.typedArray, name);
                componentsPerAttribute = property.componentCount;
                componentDatatype = Cesium.ComponentDatatype.fromTypedArray(typedArray);

                const vertexBuffer = Cesium.Buffer.createVertexBuffer({
                    context,
                    typedArray,
                    usage: Cesium.BufferUsage.STATIC_DRAW
                });

                pointCloud._geometryByteLength += vertexBuffer.sizeInBytes;

                const vertexAttribute = {
                    index: attributeLocation,
                    vertexBuffer,
                    componentsPerAttribute,
                    componentDatatype,
                    normalize: false,
                    offsetInBytes: 0,
                    strideInBytes: 0
                };

                styleableVertexAttributes.push(vertexAttribute);
                styleableShaderAttributes[name] = {
                    location: attributeLocation,
                    componentCount: componentsPerAttribute
                };
                attributeLocation += 1;
            }
        });

    }

    const positionsVertexBuffer = Cesium.Buffer.createVertexBuffer({
        context,
        typedArray: positions,
        usage: Cesium.BufferUsage.STATIC_DRAW
    });
    pointCloud._geometryByteLength += positionsVertexBuffer.sizeInBytes;

    let colorsVertexBuffer;
    if (hasColors) {
        colorsVertexBuffer = Cesium.Buffer.createVertexBuffer({
            context,
            typedArray: colors,
            usage: Cesium.BufferUsage.STATIC_DRAW
        });
        pointCloud._geometryByteLength += colorsVertexBuffer.sizeInBytes;
    }

    let normalsVertexBuffer;
    if (hasNormals) {
        normalsVertexBuffer = Cesium.Buffer.createVertexBuffer({
            context,
            typedArray: normals,
            usage: Cesium.BufferUsage.STATIC_DRAW
        });
        pointCloud._geometryByteLength += normalsVertexBuffer.sizeInBytes;
    }

    let batchIdsVertexBuffer;
    if (hasBatchIds) {
        batchIds = prepareVertexAttribute(batchIds, 'batchIds');
        batchIdsVertexBuffer = Cesium.Buffer.createVertexBuffer({
            context,
            typedArray: batchIds,
            usage: Cesium.BufferUsage.STATIC_DRAW
        });
        pointCloud._geometryByteLength += batchIdsVertexBuffer.sizeInBytes;
    }

    let attributes = [];

    if (isQuantized) {
        componentDatatype = Cesium.ComponentDatatype.UNSIGNED_SHORT;
    } else if (isQuantizedDraco) {
        componentDatatype =
            quantizedRange <= 255 ? Cesium.ComponentDatatype.UNSIGNED_BYTE : Cesium.ComponentDatatype.UNSIGNED_SHORT;
    } else {
        componentDatatype = Cesium.ComponentDatatype.FLOAT;
    }

    attributes.push({
        index: positionLocation,
        vertexBuffer: positionsVertexBuffer,
        componentsPerAttribute: 3,
        componentDatatype,
        normalize: false,
        offsetInBytes: 0,
        strideInBytes: 0
    });

    if (pointCloud._cull) {
        if (isQuantized || isQuantizedDraco) {
            pointCloud._boundingSphere = Cesium.BoundingSphere.fromCornerPoints(
                Cesium.Cartesian3.ZERO,
                pointCloud._quantizedVolumeScale
            );
        } else {
            pointCloud._boundingSphere = computeApproximateBoundingSphereFromPositions(positions);
        }
    }

    if (hasColors) {
        if (isRGB565) {
            attributes.push({
                index: colorLocation,
                vertexBuffer: colorsVertexBuffer,
                componentsPerAttribute: 1,
                componentDatatype: Cesium.ComponentDatatype.UNSIGNED_SHORT,
                normalize: false,
                offsetInBytes: 0,
                strideInBytes: 0
            });
        } else {
            const colorComponentsPerAttribute = isTranslucent ? 4 : 3;
            attributes.push({
                index: colorLocation,
                vertexBuffer: colorsVertexBuffer,
                componentsPerAttribute: colorComponentsPerAttribute,
                componentDatatype: Cesium.ComponentDatatype.UNSIGNED_BYTE,
                normalize: true,
                offsetInBytes: 0,
                strideInBytes: 0
            });
        }
    }

    if (hasNormals) {
        if (isOctEncoded16P) {
            componentsPerAttribute = 2;
            componentDatatype = Cesium.ComponentDatatype.UNSIGNED_BYTE;
        } else if (isOctEncodedDraco) {
            componentsPerAttribute = 2;
            componentDatatype =
                octEncodedRange <= 255
                    ? Cesium.ComponentDatatype.UNSIGNED_BYTE
                    : Cesium.ComponentDatatype.UNSIGNED_SHORT;
        } else {
            componentsPerAttribute = 3;
            componentDatatype = Cesium.ComponentDatatype.FLOAT;
        }
        attributes.push({
            index: normalLocation,
            vertexBuffer: normalsVertexBuffer,
            componentsPerAttribute,
            componentDatatype,
            normalize: false,
            offsetInBytes: 0,
            strideInBytes: 0
        });
    }

    if (hasBatchIds) {
        attributes.push({
            index: batchIdLocation,
            vertexBuffer: batchIdsVertexBuffer,
            componentsPerAttribute: 1,
            componentDatatype: Cesium.ComponentDatatype.fromTypedArray(batchIds),
            normalize: false,
            offsetInBytes: 0,
            strideInBytes: 0
        });
    }

    if (hasStyleableProperties) {
        attributes = attributes.concat(styleableVertexAttributes);
    }

    const vertexArray = new Cesium.VertexArray({
        context,
        attributes
    });

    const opaqueRenderState = {
        depthTest: {
            enabled: true
        }
    };

    if (pointCloud._opaquePass === Cesium.Pass.CESIUM_3D_TILE) {
        opaqueRenderState.stencilTest = Cesium.StencilConstants.setCesium3DTileBit();
        opaqueRenderState.stencilMask = Cesium.StencilConstants.CESIUM_3D_TILE_MASK;
    }

    pointCloud._opaqueRenderState = Cesium.RenderState.fromCache(opaqueRenderState);

    pointCloud._translucentRenderState = Cesium.RenderState.fromCache({
        depthTest: {
            enabled: true
        },
        depthMask: false,
        blending: Cesium.BlendingState.ALPHA_BLEND
    });

    pointCloud._drawCommand = new Cesium.DrawCommand({
        boundingVolume: new Cesium.BoundingSphere(),
        cull: pointCloud._cull,
        modelMatrix: new Cesium.Matrix4(),
        primitiveType: Cesium.PrimitiveType.POINTS,
        vertexArray,
        count: pointsLength,
        shaderProgram: undefined, // Updated in createShaders
        uniformMap: undefined, // Updated in createShaders
        renderState: isTranslucent ? pointCloud._translucentRenderState : pointCloud._opaqueRenderState,
        pass: isTranslucent ? Cesium.Pass.TRANSLUCENT : pointCloud._opaquePass,
        owner: pointCloud,
        castShadows: false,
        receiveShadows: false,
        pickId: pointCloud._pickIdLoaded()
    });
}

function createUniformMap(pointCloudParam, frameState) {
    const pointCloud = pointCloudParam;
    const { context } = frameState;
    const isQuantized = pointCloud._isQuantized;
    const isQuantizedDraco = pointCloud._isQuantizedDraco;
    const isOctEncodedDraco = pointCloud._isOctEncodedDraco;

    let uniformMap = {
        u_pointSizeAndTimeAndGeometricErrorAndDepthMultiplier() {
            const scratch = scratchPointSizeAndTimeAndGeometricErrorAndDepthMultiplier;
            scratch.x = pointCloud._attenuation ? pointCloud.maximumAttenuation : pointCloud._pointSize;
            scratch.y = pointCloud.time;

            if (pointCloud._attenuation) {
                const { frustum } = frameState.camera;
                let depthMultiplier;
                // Attenuation is maximumAttenuation in 2D/ortho
                if (frameState.mode === Cesium.SceneMode.SCENE2D || frustum instanceof Cesium.OrthographicFrustum) {
                    depthMultiplier = Number.POSITIVE_INFINITY;
                } else {
                    depthMultiplier = context.drawingBufferHeight / frameState.camera.frustum.sseDenominator;
                }

                scratch.z = pointCloud.geometricError * pointCloud.geometricErrorScale;
                scratch.w = depthMultiplier;
            }

            return scratch;
        },
        u_highlightColor() {
            return pointCloud._highlightColor;
        },
        u_constantColor() {
            return pointCloud._constantColor;
        },
        u_clippingPlanes() {
            const { clippingPlanes } = pointCloud;
            const { isClipped } = pointCloud;
            return isClipped ? clippingPlanes.texture : context.defaultTexture;
        },
        u_clippingPlanesEdgeStyle() {
            const { clippingPlanes } = pointCloud;
            if (!Cesium.defined(clippingPlanes)) {
                return Cesium.Color.TRANSPARENT;
            }

            const style = Cesium.Color.clone(clippingPlanes.edgeColor, scratchColor);
            style.alpha = clippingPlanes.edgeWidth;
            return style;
        },
        u_clippingPlanesMatrix() {
            const { clippingPlanes } = pointCloud;
            if (!Cesium.defined(clippingPlanes)) {
                return Cesium.Matrix4.IDENTITY;
            }

            const clippingPlanesOriginMatrix = Cesium.defaultValue(
                pointCloud.clippingPlanesOriginMatrix,
                pointCloud._modelMatrix
            );
            Cesium.Matrix4.multiply(
                context.uniformState.view3D,
                clippingPlanesOriginMatrix,
                scratchClippingPlaneMatrix
            );
            return Cesium.Matrix4.multiply(
                scratchClippingPlaneMatrix,
                clippingPlanes.modelMatrix,
                scratchClippingPlaneMatrix
            );
        }
    };

    if (isQuantized || isQuantizedDraco || isOctEncodedDraco) {
        uniformMap = Cesium.combine(uniformMap, {
            u_quantizedVolumeScaleAndOctEncodedRange() {
                const scratch = scratchQuantizedVolumeScaleAndOctEncodedRange;
                if (Cesium.defined(pointCloud._quantizedVolumeScale)) {
                    const scale = Cesium.Cartesian3.clone(pointCloud._quantizedVolumeScale, scratch);
                    Cesium.Cartesian3.divideByScalar(scale, pointCloud._quantizedRange, scratch);
                }
                scratch.w = pointCloud._octEncodedRange;
                return scratch;
            }
        });
    }

    if (Cesium.defined(pointCloud._uniformMapLoaded)) {
        uniformMap = pointCloud._uniformMapLoaded(uniformMap);
    }

    pointCloud._drawCommand.uniformMap = uniformMap;
}

const defaultProperties = ['POSITION', 'COLOR', 'NORMAL', 'POSITION_ABSOLUTE'];

function getStyleableProperties(source, properties) {
    // Get all the properties used by this style
    // debugger;
    const regex = /czm_tiles3d_style_(\w+)/g;
    let matches = regex.exec(source);
    while (matches !== null) {
        const name = matches[1];
        if (properties.indexOf(name) === -1) {
            properties.push(name);
        }
        matches = regex.exec(source);
    }
}

function getVertexAttribute(vertexArray, index) {
    const numberOfAttributesParam = vertexArray.numberOfAttributes;
    for (let i = 0; i < numberOfAttributesParam; i += 1) {
        const attribute = vertexArray.getAttribute(i);
        if (attribute.index === index) {
            return attribute;
        }
    }

    return undefined;
}

function modifyStyleFunction(sourceParam) {
    let source = sourceParam;
    // Replace occurrences of czm_tiles3d_style_DEFAULTPROPERTY
    const { length } = defaultProperties;
    for (let i = 0; i < length; i += 1) {
        const property = defaultProperties[i];
        const styleName = `czm_tiles3d_style_${property}`;
        const replaceName = property.toLowerCase();
        source = source.replace(new RegExp(`${styleName}(\\W)`, 'g'), `${replaceName}$1`);
    }

    // Edit the function header to accept the point position, color, and normal
    return source.replace('()', '(vec3 position, vec3 position_absolute, vec4 color, vec3 normal)');
}

function createShaders(pointCloudParam, frameState, style) {
    const pointCloud = pointCloudParam;
    let i;
    let name;
    let attribute;

    const { context } = frameState;
    const hasStyle = Cesium.defined(style);
    const isQuantized = pointCloud._isQuantized;
    const isQuantizedDraco = pointCloud._isQuantizedDraco;
    const isOctEncoded16P = pointCloud._isOctEncoded16P;
    const isOctEncodedDraco = pointCloud._isOctEncodedDraco;
    const isRGB565 = pointCloud._isRGB565;
    const isTranslucent = pointCloud._isTranslucent;
    const hasColors = pointCloud._hasColors;
    const hasNormals = pointCloud._hasNormals;
    const hasBatchIds = pointCloud._hasBatchIds;
    const backFaceCulling = pointCloud._backFaceCulling;
    const normalShading = pointCloud._normalShading;
    const { vertexArray } = pointCloud._drawCommand;
    const { clippingPlanes } = pointCloud;
    const attenuation = pointCloud._attenuation;

    let colorStyleFunction;
    let showStyleFunction;
    let pointSizeStyleFunction;
    let styleTranslucent = isTranslucent;

    if (hasStyle) {
        const shaderState = {
            translucent: false
        };
        colorStyleFunction = style.getColorShaderFunction('getColorFromStyle', 'czm_tiles3d_style_', shaderState);
        showStyleFunction = style.getShowShaderFunction('getShowFromStyle', 'czm_tiles3d_style_', shaderState);
        pointSizeStyleFunction = style.getPointSizeShaderFunction(
            'getPointSizeFromStyle',
            'czm_tiles3d_style_',
            shaderState
        );
        if (Cesium.defined(colorStyleFunction) && shaderState.translucent) {
            styleTranslucent = true;
        }
    }

    pointCloud._styleTranslucent = styleTranslucent;

    const hasColorStyle = Cesium.defined(colorStyleFunction);
    const hasShowStyle = Cesium.defined(showStyleFunction);
    const hasPointSizeStyle = Cesium.defined(pointSizeStyleFunction);
    const hasClippedContent = pointCloud.isClipped;

    // Get the properties in use by the style
    const styleableProperties = [];

    if (hasColorStyle) {
        getStyleableProperties(colorStyleFunction, styleableProperties);
        colorStyleFunction = modifyStyleFunction(colorStyleFunction);
    }
    if (hasShowStyle) {
        getStyleableProperties(showStyleFunction, styleableProperties);
        showStyleFunction = modifyStyleFunction(showStyleFunction);
    }
    if (hasPointSizeStyle) {
        getStyleableProperties(pointSizeStyleFunction, styleableProperties);
        pointSizeStyleFunction = modifyStyleFunction(pointSizeStyleFunction);
    }

    const usesColorSemantic = styleableProperties.indexOf('COLOR') >= 0;
    const usesNormalSemantic = styleableProperties.indexOf('NORMAL') >= 0;

    // Split default properties from user properties
    const userProperties = styleableProperties.filter((property) => {
        return defaultProperties.indexOf(property) === -1;
    });

    if (usesNormalSemantic && !hasNormals) {
        throw new Cesium.RuntimeError('Style references the NORMAL semantic but the point cloud does not have normals');
    }

    // Disable vertex attributes that aren't used in the style, enable attributes that are
    const styleableShaderAttributes = pointCloud._styleableShaderAttributes;
    // for (name in styleableShaderAttributes) {
    Object.getOwnPropertyNames(styleableShaderAttributes).forEach((key) => {
        // if (styleableShaderAttributes.hasOwnProperty(key)) {
        if (Object.prototype.hasOwnProperty.call(styleableShaderAttributes, key)) {
            attribute = styleableShaderAttributes[key];
            const enabled = userProperties.indexOf(key) >= 0;
            const vertexAttribute = getVertexAttribute(vertexArray, attribute.location);
            vertexAttribute.enabled = enabled;
        }
    });
    // }

    const usesColors = hasColors && (!hasColorStyle || usesColorSemantic);
    if (hasColors) {
        // Disable the color vertex attribute if the color style does not reference the color semantic
        const colorVertexAttribute = getVertexAttribute(vertexArray, colorLocation);
        colorVertexAttribute.enabled = usesColors;
    }

    const usesNormals = hasNormals && (normalShading || backFaceCulling || usesNormalSemantic);
    if (hasNormals) {
        // Disable the normal vertex attribute if normals are not used
        const normalVertexAttribute = getVertexAttribute(vertexArray, normalLocation);
        normalVertexAttribute.enabled = usesNormals;
    }

    const attributeLocations = {
        a_position: positionLocation
    };
    if (usesColors) {
        attributeLocations.a_color = colorLocation;
    }
    if (usesNormals) {
        attributeLocations.a_normal = normalLocation;
    }
    if (hasBatchIds) {
        attributeLocations.a_batchId = batchIdLocation;
    }

    let attributeDeclarations = '';

    const { length } = userProperties;
    for (i = 0; i < length; i += 1) {
        name = userProperties[i];
        attribute = styleableShaderAttributes[name];
        if (!Cesium.defined(attribute)) {
            throw new Cesium.RuntimeError(
                `Style references a property "${name}" that does not exist or is not styleable.`
            );
        }

        const { componentCount } = attribute;
        const attributeName = `czm_tiles3d_style_${name}`;
        let attributeType;
        if (componentCount === 1) {
            attributeType = 'float';
        } else {
            attributeType = `vec${componentCount}`;
        }

        attributeDeclarations += `attribute ${attributeType} ${attributeName}; \n`;
        attributeLocations[attributeName] = attribute.location;
    }

    createUniformMap(pointCloud, frameState);

    let vs =
        'attribute vec3 a_position; \n varying vec4 v_color; \n uniform vec4 u_pointSizeAndTimeAndGeometricErrorAndDepthMultiplier; \n uniform vec4 u_constantColor; \n uniform vec4 u_highlightColor; \n';
    vs += 'float u_pointSize; \n float u_time; \n';

    if (attenuation) {
        vs += 'float u_geometricError; \n float u_depthMultiplier; \n';
    }

    vs += attributeDeclarations;

    if (usesColors) {
        if (isTranslucent) {
            vs += 'attribute vec4 a_color; \n';
        } else if (isRGB565) {
            vs +=
                'attribute float a_color; \n const float SHIFT_RIGHT_11 = 1.0 / 2048.0; \n const float SHIFT_RIGHT_5 = 1.0 / 32.0; \n const float SHIFT_LEFT_11 = 2048.0; \n const float SHIFT_LEFT_5 = 32.0; \n const float NORMALIZE_6 = 1.0 / 64.0; \n const float NORMALIZE_5 = 1.0 / 32.0; \n';
        } else {
            vs += 'attribute vec3 a_color; \n';
        }
    }
    if (usesNormals) {
        if (isOctEncoded16P || isOctEncodedDraco) {
            vs += 'attribute vec2 a_normal; \n';
        } else {
            vs += 'attribute vec3 a_normal; \n';
        }
    }

    if (hasBatchIds) {
        vs += 'attribute float a_batchId; \n';
    }

    if (isQuantized || isQuantizedDraco || isOctEncodedDraco) {
        vs += 'uniform vec4 u_quantizedVolumeScaleAndOctEncodedRange; \n';
    }

    if (hasColorStyle) {
        vs += colorStyleFunction;
    }

    if (hasShowStyle) {
        vs += showStyleFunction;
    }

    if (hasPointSizeStyle) {
        vs += pointSizeStyleFunction;
    }

    vs +=
        'void main() \n { \n     u_pointSize = u_pointSizeAndTimeAndGeometricErrorAndDepthMultiplier.x; \n     u_time = u_pointSizeAndTimeAndGeometricErrorAndDepthMultiplier.y; \n';

    if (attenuation) {
        vs +=
            '    u_geometricError = u_pointSizeAndTimeAndGeometricErrorAndDepthMultiplier.z; \n     u_depthMultiplier = u_pointSizeAndTimeAndGeometricErrorAndDepthMultiplier.w; \n';
    }

    if (usesColors) {
        if (isTranslucent) {
            vs += '    vec4 color = a_color; \n';
        } else if (isRGB565) {
            vs +=
                '    float compressed = a_color; \n     float r = floor(compressed * SHIFT_RIGHT_11); \n     compressed -= r * SHIFT_LEFT_11; \n     float g = floor(compressed * SHIFT_RIGHT_5); \n     compressed -= g * SHIFT_LEFT_5; \n     float b = compressed; \n     vec3 rgb = vec3(r * NORMALIZE_5, g * NORMALIZE_6, b * NORMALIZE_5); \n     vec4 color = vec4(rgb, 1.0); \n';
        } else {
            vs += '    vec4 color = vec4(a_color, 1.0); \n';
        }
    } else {
        vs += '    vec4 color = u_constantColor; \n';
    }

    if (isQuantized || isQuantizedDraco) {
        vs += '    vec3 position = a_position * u_quantizedVolumeScaleAndOctEncodedRange.xyz; \n';
    } else {
        vs += '    vec3 position = a_position; \n';
    }
    vs += '    vec3 position_absolute = vec3(czm_model * vec4(position, 1.0)); \n';

    if (usesNormals) {
        if (isOctEncoded16P) {
            vs += '    vec3 normal = czm_octDecode(a_normal); \n';
        } else if (isOctEncodedDraco) {
            // Draco oct-encoding decodes to zxy order
            vs += '    vec3 normal = czm_octDecode(a_normal, u_quantizedVolumeScaleAndOctEncodedRange.w).zxy; \n';
        } else {
            vs += '    vec3 normal = a_normal; \n';
        }
        vs += '    vec3 normalEC = czm_normal * normal; \n';
    } else {
        vs += '    vec3 normal = vec3(1.0); \n';
    }

    if (hasColorStyle) {
        vs += '    color = getColorFromStyle(position, position_absolute, color, normal); \n';
    }

    if (hasShowStyle) {
        vs += '    float show = float(getShowFromStyle(position, position_absolute, color, normal)); \n';
    }

    if (hasPointSizeStyle) {
        vs += '    gl_PointSize = getPointSizeFromStyle(position, position_absolute, color, normal); \n';
    } else if (attenuation) {
        vs +=
            '    vec4 positionEC = czm_modelView * vec4(position, 1.0); \n' +
            '    float depth = -positionEC.z; \n' +
            // compute SSE for this point
            '    gl_PointSize = min((u_geometricError / depth) * u_depthMultiplier, u_pointSize); \n';
    } else {
        vs += '    gl_PointSize = u_pointSize; \n';
    }

    vs += '    color = color * u_highlightColor; \n';

    if (usesNormals && normalShading) {
        vs +=
            '    float diffuseStrength = czm_getLambertDiffuse(czm_sunDirectionEC, normalEC); \n' +
            '    diffuseStrength = max(diffuseStrength, 0.4); \n' + // Apply some ambient lighting
            '    color.xyz *= diffuseStrength; \n';
    }

    vs += '    v_color = color; \n     gl_Position = czm_modelViewProjection * vec4(position, 1.0); \n';

    if (usesNormals && backFaceCulling) {
        vs +=
            '    float visible = step(-normalEC.z, 0.0); \n     gl_Position *= visible; \n     gl_PointSize *= visible; \n';
    }

    if (hasShowStyle) {
        vs += '    gl_Position *= show; \n     gl_PointSize *= show; \n';
    }

    vs += '} \n';

    let fs = 'varying vec4 v_color; \n';

    if (hasClippedContent) {
        fs +=
            'uniform sampler2D u_clippingPlanes; \n uniform mat4 u_clippingPlanesMatrix; \n uniform vec4 u_clippingPlanesEdgeStyle; \n';
        fs += '\n';
        fs += getClippingFunction(clippingPlanes, context);
        fs += '\n';
    }

    fs += 'void main() \n { \n     gl_FragColor = czm_gammaCorrect(v_color); \n';

    if (hasClippedContent) {
        fs += getClipAndStyleCode('u_clippingPlanes', 'u_clippingPlanesMatrix', 'u_clippingPlanesEdgeStyle');
    }

    fs += '} \n';

    if (Cesium.defined(pointCloud._vertexShaderLoaded)) {
        vs = pointCloud._vertexShaderLoaded(vs);
    }

    if (Cesium.defined(pointCloud._fragmentShaderLoaded)) {
        fs = pointCloud._fragmentShaderLoaded(fs);
    }

    const drawCommand = pointCloud._drawCommand;
    if (Cesium.defined(drawCommand.shaderProgram)) {
        // Destroy the old shader
        drawCommand.shaderProgram.destroy();
    }
    drawCommand.shaderProgram = Cesium.ShaderProgram.fromCache({
        context,
        vertexShaderSource: vs,
        fragmentShaderSource: fs,
        attributeLocations
    });

    try {
        // Check if the shader compiles correctly. If not there is likely a syntax error with the style.
        drawCommand.shaderProgram._bind();
    } catch (error) {
        // Rephrase the error.
        throw new Cesium.RuntimeError(
            'Error generating style shader: this may be caused by a type mismatch, index out-of-bounds, or other syntax error.'
        );
    }
}

function decodeDraco(pointCloudParam, context) {
    const pointCloud = pointCloudParam;
    if (pointCloud._decodingState === DecodingState.READY) {
        return false;
    }
    if (pointCloud._decodingState === DecodingState.NEEDS_DECODE) {
        const parsedContent = pointCloud._parsedContent;
        const { draco } = parsedContent;
        const decodePromise = Cesium.DracoLoader.decodePointCloud(draco, context);
        if (Cesium.defined(decodePromise)) {
            pointCloud._decodingState = DecodingState.DECODING;
            decodePromise
                .then((result) => {
                    pointCloud._decodingState = DecodingState.READY;
                    const decodedPositions = Cesium.defined(result.POSITION) ? result.POSITION.array : undefined;
                    const decodedRgb = Cesium.defined(result.RGB) ? result.RGB.array : undefined;
                    const decodedRgba = Cesium.defined(result.RGBA) ? result.RGBA.array : undefined;
                    const decodedNormals = Cesium.defined(result.NORMAL) ? result.NORMAL.array : undefined;
                    const decodedBatchIds = Cesium.defined(result.BATCH_ID) ? result.BATCH_ID.array : undefined;
                    const isQuantizedDraco =
                        Cesium.defined(decodedPositions) && Cesium.defined(result.POSITION.data.quantization);
                    const isOctEncodedDraco =
                        Cesium.defined(decodedNormals) && Cesium.defined(result.NORMAL.data.quantization);
                    if (isQuantizedDraco) {
                        // Draco quantization range == quantized volume scale - size in meters of the quantized volume
                        // Internal quantized range is the range of values of the quantized data, e.g. 255 for 8-bit, 1023 for 10-bit, etc
                        const { quantization } = result.POSITION.data;
                        const { range } = quantization;
                        pointCloud._quantizedVolumeScale = Cesium.Cartesian3.fromElements(range, range, range);
                        pointCloud._quantizedVolumeOffset = Cesium.Cartesian3.unpack(quantization.minValues);
                        pointCloud._quantizedRange = 2 ** quantization.quantizationBits - 1.0;
                        pointCloud._isQuantizedDraco = true;
                    }
                    if (isOctEncodedDraco) {
                        pointCloud._octEncodedRange = 2 ** result.NORMAL.data.quantization.quantizationBits - 1.0;
                        pointCloud._isOctEncodedDraco = true;
                    }
                    let { styleableProperties } = parsedContent;
                    const { batchTableProperties } = draco;
                    // for (const name in batchTableProperties) {
                    Object.getOwnPropertyNames(batchTableProperties).forEach((name) => {
                        // if (batchTableProperties.hasOwnProperty(name)) {
                        if (Object.prototype.hasOwnProperty.call(batchTableProperties, name)) {
                            const property = result[name];
                            if (!Cesium.defined(styleableProperties)) {
                                styleableProperties = {};
                            }
                            styleableProperties[name] = {
                                typedArray: property.array,
                                componentCount: property.data.componentsPerAttribute
                            };
                        }
                    });
                    // }
                    parsedContent.positions = Cesium.defaultValue(decodedPositions, parsedContent.positions);
                    parsedContent.colors = Cesium.defaultValue(
                        Cesium.defaultValue(decodedRgba, decodedRgb),
                        parsedContent.colors
                    );
                    parsedContent.normals = Cesium.defaultValue(decodedNormals, parsedContent.normals);
                    parsedContent.batchIds = Cesium.defaultValue(decodedBatchIds, parsedContent.batchIds);
                    parsedContent.styleableProperties = styleableProperties;
                })
                .otherwise((error) => {
                    pointCloud._decodingState = DecodingState.FAILED;
                    pointCloud._readyPromise.reject(error);
                });
        }
    }
    return true;
}

const scratchComputedTranslation = new Cesium.Cartesian4();
const scratchScale = new Cesium.Cartesian3();

// -->>

export default class MapGISM3DPointCloud {
    constructor(options) {
        // >>includeStart('debug', pragmas.debug);
        Cesium.Check.typeOf.object('options', options);
        Cesium.Check.typeOf.object('options.arrayBuffer', options.param.arrayBuffer);
        // >>includeEnd('debug');

        // Hold onto the payload until the render resources are created
        this._parsedContent = undefined;

        this._drawCommand = undefined;
        this._isTranslucent = false;
        this._styleTranslucent = false;
        this._constantColor = Cesium.Color.clone(Cesium.Color.DARKGRAY);
        this._highlightColor = Cesium.Color.clone(Cesium.Color.WHITE);
        this._pointSize = 1.0;

        this._rtcCenter = undefined;
        this._quantizedVolumeScale = undefined;
        this._quantizedVolumeOffset = undefined;

        // These values are used to regenerate the shader when the style changes
        this._styleableShaderAttributes = undefined;
        this._isQuantized = false;
        this._isOctEncoded16P = false;
        this._isRGB565 = false;
        this._hasColors = false;
        this._hasNormals = false;
        this._hasBatchIds = false;

        // Draco
        this._decodingState = DecodingState.READY;
        this._dequantizeInShader = true;
        this._isQuantizedDraco = false;
        this._isOctEncodedDraco = false;
        this._quantizedRange = 0.0;
        this._octEncodedRange = 0.0;

        // Use per-point normals to hide back-facing points.
        this.backFaceCulling = false;
        this._backFaceCulling = false;

        // Whether to enable normal shading
        this.normalShading = true;
        this._normalShading = true;

        this._opaqueRenderState = undefined;
        this._translucentRenderState = undefined;

        this._mode = undefined;

        this._ready = false;
        this._readyPromise = Cesium.when.defer();
        this._pointsLength = 0;
        this._geometryByteLength = 0;

        this._vertexShaderLoaded = options.vertexShaderLoaded;
        this._fragmentShaderLoaded = options.fragmentShaderLoaded;
        this._uniformMapLoaded = options.uniformMapLoaded;
        this._batchTableLoaded = options.batchTableLoaded;
        this._pickIdLoaded = options.pickIdLoaded;
        this._opaquePass = Cesium.defaultValue(options.opaquePass, Cesium.Pass.OPAQUE);
        this._cull = Cesium.defaultValue(options.cull, true);

        this.style = undefined;
        this._style = undefined;
        this.styleDirty = false;

        this.modelMatrix = Cesium.Matrix4.clone(Cesium.Matrix4.IDENTITY);
        this._modelMatrix = Cesium.Matrix4.clone(Cesium.Matrix4.IDENTITY);

        this.time = 0.0; // For styling
        this.shadows = Cesium.ShadowMode.ENABLED;
        this._boundingSphere = undefined;

        this.clippingPlanes = undefined;
        this.isClipped = false;
        this.clippingPlanesDirty = false;
        // If defined, use this matrix to position the clipping planes instead of the modelMatrix.
        // This is so that when point clouds are part of a tileset they all get clipped relative
        // to the root tile.
        this.clippingPlanesOriginMatrix = undefined;

        this.attenuation = false;
        this._attenuation = false;

        // Options for geometric error based attenuation
        this.geometricError = 0.0;
        this.geometricErrorScale = 1.0;
        this.maximumAttenuation = this._pointSize;

        initialize(this, options);
    }

    get pointsLength() {
        return this._pointsLength;
    }

    get geometryByteLength() {
        return this._geometryByteLength;
    }

    get ready() {
        return this._ready;
    }

    get readyPromise() {
        return this._readyPromise.promise;
    }

    get color() {
        return Cesium.Color.clone(this._highlightColor);
    }

    set color(value) {
        this._highlightColor = Cesium.Color.clone(value, this._highlightColor);
    }

    get boundingSphere() {
        if (Cesium.defined(this._drawCommand)) {
            return this._drawCommand.boundingVolume;
        }
        return undefined;
    }

    set boundingSphere(value) {
        this._boundingSphere = Cesium.BoundingSphere.clone(value);
    }

    update(frameState) {
        const { context } = frameState;
        const decoding = decodeDraco(this, context);
        if (decoding) {
            return;
        }

        let shadersDirty = false;
        let modelMatrixDirty = !Cesium.Matrix4.equals(this._modelMatrix, this.modelMatrix);

        if (this._mode !== frameState.mode) {
            this._mode = frameState.mode;
            modelMatrixDirty = true;
        }

        if (!Cesium.defined(this._drawCommand)) {
            createResources(this, frameState);
            modelMatrixDirty = true;
            shadersDirty = true;
            this._ready = true;
            this._readyPromise.resolve(this);
            this._parsedContent = undefined; // Unload
        }

        if (modelMatrixDirty) {
            Cesium.Matrix4.clone(this.modelMatrix, this._modelMatrix);
            const { modelMatrix } = this._drawCommand;
            Cesium.Matrix4.clone(this._modelMatrix, modelMatrix);

            if (Cesium.defined(this._rtcCenter)) {
                Cesium.Matrix4.multiplyByTranslation(modelMatrix, this._rtcCenter, modelMatrix);
            }
            if (Cesium.defined(this._quantizedVolumeOffset)) {
                Cesium.Matrix4.multiplyByTranslation(modelMatrix, this._quantizedVolumeOffset, modelMatrix);
            }

            if (frameState.mode !== Cesium.SceneMode.SCENE3D) {
                const projection = frameState.mapProjection;
                const translation = Cesium.Matrix4.getColumn(modelMatrix, 3, scratchComputedTranslation);
                if (!Cesium.Cartesian4.equals(translation, Cesium.Cartesian4.UNIT_W)) {
                    Cesium.Transforms.basisTo2D(projection, modelMatrix, modelMatrix);
                }
            }

            const boundingSphere = this._drawCommand.boundingVolume;
            Cesium.BoundingSphere.clone(this._boundingSphere, boundingSphere);

            if (this._cull) {
                const { center } = boundingSphere;
                Cesium.Matrix4.multiplyByPoint(modelMatrix, center, center);
                const scale = Cesium.Matrix4.getScale(modelMatrix, scratchScale);
                boundingSphere.radius *= Cesium.Cartesian3.maximumComponent(scale);
            }
        }

        if (this.clippingPlanesDirty) {
            this.clippingPlanesDirty = false;
            shadersDirty = true;
        }

        if (this._attenuation !== this.attenuation) {
            this._attenuation = this.attenuation;
            shadersDirty = true;
        }

        if (this.backFaceCulling !== this._backFaceCulling) {
            this._backFaceCulling = this.backFaceCulling;
            shadersDirty = true;
        }

        if (this.normalShading !== this._normalShading) {
            this._normalShading = this.normalShading;
            shadersDirty = true;
        }

        if (this._style !== this.style || this.styleDirty) {
            this._style = this.style;
            this.styleDirty = false;
            shadersDirty = true;
        }

        if (shadersDirty) {
            createShaders(this, frameState, this._style);
        }

        this._drawCommand.castShadows = Cesium.ShadowMode.castShadows(this.shadows);
        this._drawCommand.receiveShadows = Cesium.ShadowMode.receiveShadows(this.shadows);

        // Update the render state
        const isTranslucent =
            this._highlightColor.alpha < 1.0 || this._constantColor.alpha < 1.0 || this._styleTranslucent;
        this._drawCommand.renderState = isTranslucent ? this._translucentRenderState : this._opaqueRenderState;
        this._drawCommand.pass = isTranslucent ? Cesium.Pass.TRANSLUCENT : this._opaquePass;

        const { commandList } = frameState;

        const { passes } = frameState;
        if (passes.render || passes.pick) {
            commandList.push(this._drawCommand);
        }
    }

    // eslint-disable-next-line class-methods-use-this
    isDestroyed() {
        return false;
    }

    destroy() {
        const command = this._drawCommand;
        if (Cesium.defined(command)) {
            command.vertexArray = command.vertexArray && command.vertexArray.destroy();
            command.shaderProgram = command.shaderProgram && command.shaderProgram.destroy();
        }
        return Cesium.destroyObject(this);
    }
}

CesiumZondy.M3D.MapGISM3DPointCloud = MapGISM3DPointCloud;
