import { CesiumZondy } from '../core/Base';
import CreateIndexBufferJob from  './CreateIndexBufferJob';
import CreateVertexBufferJob  from  './CreateVertexBufferJob';
import CachedRendererResources from './CachedRendererResources';

    let boundingSphereCartesian3Scratch = new Cesium.Cartesian3();

    let ModelState = Cesium.ModelUtility.ModelState;

    function setCachedGltf(model, cachedGltf) {
        model._cachedGltf = cachedGltf;
    }

    let gltfCache = {};
    let uriToGuid = {};

    class CachedGltf {
            constructor(optionsParam){
            const options = Cesium.defaultValue(optionsParam, Cesium.defaultValue.EMPTY_OBJECT);
            this._gltf = options.gltf;
            this.ready = options.ready;
            this.modelsToLoad = [];
            this.count = 0;
        }

        set gltf(value){
            this._gltf = value;
        }

        get gltf(){
            return this._gltf;
        }
    }

    CachedGltf.prototype.makeReady = function(gltfJson) {
        this.gltf = gltfJson;

        let models = this.modelsToLoad;
        let length = models.length;
        for (let i = 0; i < length; ++i) {
            let m = models[i];
            if (!m.isDestroyed()) {
                setCachedGltf(m, this);
            }
        }
        this.modelsToLoad = undefined;
        this.ready = true;
    };


export default class M3DModelParser{
    constructor(optionsParam){
        debugger;
        const options = Cesium.defaultValue(optionsParam, Cesium.defaultValue.EMPTY_OBJECT);
        let cacheKey = options.cacheKey;
        this._cacheKey = cacheKey;
        this._cachedGltf = undefined;
        this._releaseGltfJson = Cesium.defaultValue(options.releaseGltfJson, false);

        let cachedGltf;
        if (Cesium.defined(cacheKey) && Cesium.defined(gltfCache[cacheKey]) && gltfCache[cacheKey].ready) {
            cachedGltf = gltfCache[cacheKey];
            ++cachedGltf.count;
        } else {
            let gltf = options.gltf;

            if (Cesium.defined(gltf)) {
                if (gltf instanceof ArrayBuffer) {
                    gltf = new Uint8Array(gltf);
                }

                if (gltf instanceof Uint8Array) {
                    // Binary glTF
                    let parsedGltf = Cesium.parseGlb(gltf);
                    cachedGltf = new CachedGltf({
                        gltf : parsedGltf,
                        ready : true
                    });
                } else {
                    // Normal glTF (JSON)
                    cachedGltf = new CachedGltf({
                        gltf : options.gltf,
                        ready : true
                    });
                }

                cachedGltf.count = 1;
                let constraintRegions = options.constraintRegions;
                let  isPlaneViewer = Cesium.defined(options.isPlaneViewer, false);
                if (Cesium.defined(cacheKey)) {
                    gltfCache[cacheKey] = cachedGltf;
                }
            }
        }
        setCachedGltf(this, cachedGltf);

        let basePath = Cesium.defaultValue(options.basePath, '');
        this._resource = Cesium.Resource.createIfNeeded(basePath);

        this.show = Cesium.defaultValue(options.show, true);

        /**
         * The silhouette color.
         *
         * @type {Color}
         *
         * @default Color.RED
         */
        this.silhouetteColor = Cesium.defaultValue(options.silhouetteColor, Cesium.Color.RED);
        this._silhouetteColor = new Cesium.Color();
        this._silhouetteColorPreviousAlpha = 1.0;
        this._normalAttributeName = undefined;

        /**
         * The size of the silhouette in pixels.
         *
         * @type {Number}
         *
         * @default 0.0
         */
        this.silhouetteSize = Cesium.defaultValue(options.silhouetteSize, 0.0);

        /**
         * The 4x4 transformation matrix that transforms the model from model to world coordinates.
         * When this is the identity matrix, the model is drawn in world coordinates, i.e., Earth's WGS84 coordinates.
         * Local reference frames can be used by providing a different transformation matrix, like that returned
         * by {@link Transforms.eastNorthUpToFixedFrame}.
         *
         * @type {Matrix4}
         *
         * @default {@link Matrix4.IDENTITY}
         *
         * @example
         * let origin = Cesium.Cartesian3.fromDegrees(-95.0, 40.0, 200000.0);
         * m.modelMatrix = Cesium.Transforms.eastNorthUpToFixedFrame(origin);
         */
        this.modelMatrix = Cesium.Matrix4.clone(Cesium.defaultValue(options.modelMatrix, Cesium.Matrix4.IDENTITY));
        this._modelMatrix = Cesium.Matrix4.clone(this.modelMatrix);
        this._clampedModelMatrix = undefined;

        /**
         * A uniform scale applied to this model before the {@link Model#modelMatrix}.
         * Values greater than <code>1.0</code> increase the size of the model; values
         * less than <code>1.0</code> decrease.
         *
         * @type {Number}
         *
         * @default 1.0
         */
        this.scale = Cesium.defaultValue(options.scale, 1.0);
        this._scale = this.scale;

        /**
         * The approximate minimum pixel size of the model regardless of zoom.
         * This can be used to ensure that a model is visible even when the viewer
         * zooms out.  When <code>0.0</code>, no minimum size is enforced.
         *
         * @type {Number}
         *
         * @default 0.0
         */
        this.minimumPixelSize = Cesium.defaultValue(options.minimumPixelSize, 0.0);
        this._minimumPixelSize = this.minimumPixelSize;

        /**
         * The maximum scale size for a model. This can be used to give
         * an upper limit to the {@link Model#minimumPixelSize}, ensuring that the model
         * is never an unreasonable scale.
         *
         * @type {Number}
         */
        this.maximumScale = options.maximumScale;
        this._maximumScale = this.maximumScale;

        /**
         * User-defined object returned when the model is picked.
         *
         * @type Object
         *
         * @default undefined
         *
         * @see Scene#pick
         */
        this.id = options.id;
        this._id = options.id;

        /**
         * Returns the height reference of the model
         *
         * @type {HeightReference}
         *
         * @default HeightReference.NONE
         */
        this.heightReference = Cesium.defaultValue(options.heightReference, Cesium.HeightReference.NONE);
        this._heightReference = this.heightReference;
        this._heightChanged = false;
        this._removeUpdateHeightCallback = undefined;
        let scene = options.scene;
        this._scene = scene;
        if (Cesium.defined(scene) && Cesium.defined(scene.terrainProviderChanged)) {
            this._terrainProviderChangedCallback = scene.terrainProviderChanged.addEventListener(function() {
                this._heightChanged = true;
            }, this);
        }

        /**
         * Used for picking primitives that wrap a model.
         *
         * @private
         */
        this._pickObject = options.pickObject;
        this._allowPicking = Cesium.defaultValue(options.allowPicking, true);

        this._ready = false;
        this._readyPromise = Cesium.when.defer();

        /**
         * The currently playing glTF animations.
         *
         * @type {ModelAnimationCollection}
         */
        this.activeAnimations = new Cesium.ModelAnimationCollection(this);

        /**
         * Determines if the model's animations should hold a pose over frames where no keyframes are specified.
         *
         * @type {Boolean}
         */
        this.clampAnimations = Cesium.defaultValue(options.clampAnimations, true);

        this._defaultTexture = undefined;
        this._incrementallyLoadTextures = Cesium.defaultValue(options.incrementallyLoadTextures, true);
        this._asynchronous = Cesium.defaultValue(options.asynchronous, true);

        /**
         * Determines whether the model casts or receives shadows from each light source.
         *
         * @type {ShadowMode}
         *
         * @default ShadowMode.ENABLED
         */
        this.shadows = Cesium.defaultValue(options.shadows, Cesium.ShadowMode.ENABLED);
        this._shadows = this.shadows;

        /**
         * A color that blends with the model's rendered color.
         *
         * @type {Color}
         *
         * @default Color.WHITE
         */
        this.color = Cesium.defaultValue(options.color, Cesium.Color.WHITE);
        this._color = new Cesium.Color();
        this._colorPreviousAlpha = 1.0;

        /**
         * Defines how the color blends with the model.
         *
         * @type {ColorBlendMode}
         *
         * @default ColorBlendMode.HIGHLIGHT
         */
        this.colorBlendMode = Cesium.defaultValue(options.colorBlendMode, Cesium.ColorBlendMode.HIGHLIGHT);

    
        this.colorBlendAmount = Cesium.defaultValue(options.colorBlendAmount, 0.5);

        this._colorShadingEnabled = false;

        this._clippingPlanes = undefined;
        this.clippingPlanes = options.clippingPlanes;
       
        this._clippingPlanesState = 0;
      
        this.clippingPlanesOriginMatrix = undefined;

  
        this.debugShowBoundingVolume = Cesium.defaultValue(options.debugShowBoundingVolume, false);
        this._debugShowBoundingVolume = false;

        this.debugWireframe = Cesium.defaultValue(options.debugWireframe, false);
        this._debugWireframe = false;

        this._distanceDisplayCondition = options.distanceDisplayCondition;

        // Undocumented options
        this._addBatchIdToGeneratedShaders = options.addBatchIdToGeneratedShaders;
        this._precreatedAttributes = options.precreatedAttributes;
        this._vertexShaderLoaded = options.vertexShaderLoaded;
        this._fragmentShaderLoaded = options.fragmentShaderLoaded;
        this._uniformMapLoaded = options.uniformMapLoaded;
        this._pickIdLoaded = options.pickIdLoaded;

        this._ignoreCommands = Cesium.defaultValue(options.ignoreCommands, false);
        this._requestType = options.requestType;
        this._upAxis = Cesium.defaultValue(options.upAxis,Cesium.Axis.Y);
        this._gltfForwardAxis = Cesium.Axis.Z;
        this._forwardAxis = options.forwardAxis;

        /**
         * @private
         * @readonly
         */
        this.cull = Cesium.defaultValue(options.cull, true);

        /**
         * @private
         * @readonly
         */
        this.opaquePass = Cesium.defaultValue(options.opaquePass, Cesium.Pass.OPAQUE);

        this._computedModelMatrix = new Cesium.Matrix4(); // Derived from modelMatrix and scale
        this._clippingPlaneModelViewMatrix = Cesium.Matrix4.clone(Cesium.Matrix4.IDENTITY); // Derived from modelMatrix, scale, and the current view matrix
        this._initialRadius = undefined;           // Radius without model's scale property, model-matrix scale, animations, or skins
        this._boundingSphere = undefined;
        this._scaledBoundingSphere = new Cesium.BoundingSphere();
        this._state = ModelState.NEEDS_LOAD;
        this._loadResources = undefined;

        this._mode = undefined;

        this._perNodeShowDirty = false;            // true when the Cesium API was used to change a node's show property
        this._cesiumAnimationsDirty = false;       // true when the Cesium API, not a glTF animation, changed a node transform
        this._dirty = false;                       // true when the model was transformed this frame
        this._maxDirtyNumber = 0;                  // Used in place of a dirty boolean flag to avoid an extra graph traversal

        this._runtime = {
            animations : undefined,
            articulationsByName : undefined,
            articulationsByStageKey : undefined,
            stagesByKey : undefined,
            rootNodes : undefined,
            nodes : undefined,            // Indexed with the node's index
            nodesByName : undefined,      // Indexed with name property in the node
            skinnedNodes : undefined,
            meshesByName : undefined,     // Indexed with the name property in the mesh
            materialsByName : undefined,  // Indexed with the name property in the material
            materialsById : undefined     // Indexed with the material's index
        };

        this._uniformMaps = {};           // Not cached since it can be targeted by glTF animation
        this._extensionsUsed = undefined;     // Cached used glTF extensions
        this._extensionsRequired = undefined; // Cached required glTF extensions
        this._quantizedUniforms = {};     // Quantized uniforms for each program for WEB3D_quantized_attributes
        this._programPrimitives = {};
        this._rendererResources = {       // Cached between models with the same url/cache-key
            buffers : {},
            vertexArrays : {},
            programs : {},
            sourceShaders : {},
            silhouettePrograms : {},
            textures : {},
            samplers : {},
            renderStates : {}
        };
        this._cachedRendererResources = undefined;
        this._loadRendererResourcesFromCache = false;

        this._dequantizeInShader = Cesium.defaultValue(options.dequantizeInShader, true);
        this._decodedData = {};

        this._cachedGeometryByteLength = 0;
        this._cachedTexturesByteLength = 0;
        this._geometryByteLength = 0;
        this._texturesByteLength = 0;
        this._trianglesLength = 0;

        // Hold references for shader reconstruction.
        // Hold these separately because _cachedGltf may get released (this.releaseGltfJson)
        this._sourceTechniques = {};
        this._sourcePrograms = {};
        this._quantizedVertexShaders = {};

        this._nodeCommands = [];
        this._pickIds = [];

        // CESIUM_RTC extension
        this._rtcCenter = undefined;    // reference to either 3D or 2D
        this._rtcCenterEye = undefined; // in eye coordinates
        this._rtcCenter3D = undefined;  // in world coordinates
        this._rtcCenter2D = undefined;  // in projected world coordinates

        this._sourceVersion = undefined;
        this._sourceKHRTechniquesWebGL = undefined;

        this._imageBasedLightingFactor = new Cesium.Cartesian2(1.0, 1.0);
        Cesium.Cartesian2.clone(options.imageBasedLightingFactor, this._imageBasedLightingFactor);
        this._lightColor = Cesium.Cartesian3.clone(options.lightColor);

        this._luminanceAtZenith = undefined;
        this.luminanceAtZenith = Cesium.defaultValue(options.luminanceAtZenith, 0.5);

        this._sphericalHarmonicCoefficients = options.sphericalHarmonicCoefficients;
        this._specularEnvironmentMaps = options.specularEnvironmentMaps;
        this._shouldUpdateSpecularMapAtlas = true;
        this._specularEnvironmentMapAtlas = undefined;

        this._useDefaultSphericalHarmonics = false;
        this._useDefaultSpecularMaps = false;

        this._shouldRegenerateShaders = false;
    }

    get gltf(){
        return Cesium.defined(this._cachedGltf) ? this._cachedGltf.gltf : undefined;
    }

    get releaseGltfJson(){
         return this._releaseGltfJson;
    }

    get cacheKey(){
        return this._cacheKey;
    }

    get basePath(){
        return this._resource.url;
    }

    get boundingSphere(){
         if (this._state !== ModelState.LOADED) {
                    throw new Cesium.DeveloperError('The model is not loaded.  Use Model.readyPromise or wait for Model.ready to be true.');
                }

                let modelMatrix = this.modelMatrix;
                if ((this.heightReference !== Cesium.HeightReference.NONE) && this._clampedModelMatrix) {
                    modelMatrix = this._clampedModelMatrix;
                }

                let nonUniformScale = Cesium.Matrix4.getScale(modelMatrix, boundingSphereCartesian3Scratch);
                let scale = Cesium.defined(this.maximumScale) ? Cesium.Math.min(this.maximumScale, this.scale) : this.scale;
                Cartesian3.multiplyByScalar(nonUniformScale, scale, nonUniformScale);

                let scaledBoundingSphere = this._scaledBoundingSphere;
                scaledBoundingSphere.center = Cesium.Cartesian3.multiplyComponents(this._boundingSphere.center, nonUniformScale, scaledBoundingSphere.center);
                scaledBoundingSphere.radius = Cesium.Cartesian3.maximumComponent(nonUniformScale) * this._initialRadius;

                if (Cesium.defined(this._rtcCenter)) {
                    Cesium.Cartesian3.add(this._rtcCenter, scaledBoundingSphere.center, scaledBoundingSphere.center);
                }

                return scaledBoundingSphere;
    }

    get ready(){
        return this._ready;
    }

    get readyPromise(){
        return this._readyPromise.promise;
    }

    get asynchronous(){
        return this._asynchronous;
    } 

    get allowPicking(){
         return this._allowPicking;
    }

    get incrementallyLoadTextures(){
        return this._incrementallyLoadTextures;
    }

    get pendingTextureLoads(){
         return Cesium.defined(this._loadResources) ? this._loadResources.pendingTextureLoads : 0;
    }

    get dirty(){
        return this._dirty;
    }

    get distanceDisplayCondition(){
         return this._distanceDisplayCondition;
    }

    set distanceDisplayCondition(value){
        if (Cesium.defined(value) && value.far <= value.near) {
            throw new Cesium.DeveloperError('far must be greater than near');
        }
        //>>includeEnd('debug');
        this._distanceDisplayCondition = Cesium.DistanceDisplayCondition.clone(value, this._distanceDisplayCondition);
    }

    get extensionsUsed(){
         if (!Cesium.defined(this._extensionsUsed)) {
            this._extensionsUsed = Cesium.ModelUtility.getUsedExtensions(this.gltf);
        }
        return this._extensionsUsed;
    }

    get extensionsRequired(){
        if (!Cesium.defined(this._extensionsRequired)) {
            this._extensionsRequired = Cesium.ModelUtility.getRequiredExtensions(this.gltf);
        }
        return this._extensionsRequired;
    }

    get upAxis(){
         return this._upAxis;
    }

    get forwardAxis(){
        if (Cesium.defined(this._forwardAxis)) {
            return this._forwardAxis;
        }
        return this._gltfForwardAxis;
    }

    get trianglesLength(){
         return this._trianglesLength;
    }
       
    get geometryByteLength(){
        return this._geometryByteLength;
    }

    get texturesByteLength(){
        return this._texturesByteLength;
    }

    get cachedGeometryByteLength(){
         return this._cachedGeometryByteLength;
    }

    get cachedTexturesByteLength(){
         return this._cachedTexturesByteLength;
    }

    get clippingPlanes(){
        return this._clippingPlanes;
    }

    set clippingPlanes(value){
        if (value === this._clippingPlanes) {
            return;
        }
        // Handle destroying, checking of unknown, checking for existing ownership
        Cesium.ClippingPlaneCollection.setOwner(value, this, '_clippingPlanes');
    }

    get pickIds(){
         return this._pickIds;
    }

    get imageBasedLightingFactor(){
        return this._imageBasedLightingFactor;
    }

    set imageBasedLightingFactor(value){
         //>>includeStart('debug', pragmas.debug);
        Check.typeOf.object('imageBasedLightingFactor', value);
        Check.typeOf.number.greaterThanOrEquals('imageBasedLightingFactor.x', value.x, 0.0);
        Check.typeOf.number.lessThanOrEquals('imageBasedLightingFactor.x', value.x, 1.0);
        Check.typeOf.number.greaterThanOrEquals('imageBasedLightingFactor.y', value.y, 0.0);
        Check.typeOf.number.lessThanOrEquals('imageBasedLightingFactor.y', value.y, 1.0);
        //>>includeEnd('debug');
        let imageBasedLightingFactor = this._imageBasedLightingFactor;
        if ((value === imageBasedLightingFactor) || Cesium.Cartesian2.equals(value, imageBasedLightingFactor)) {
            return;
        }
        this._shouldRegenerateShaders = this._shouldRegenerateShaders || (this._imageBasedLightingFactor.x > 0.0 && value.x === 0.0) || (this._imageBasedLightingFactor.x === 0.0 && value.x > 0.0);
        this._shouldRegenerateShaders = this._shouldRegenerateShaders || (this._imageBasedLightingFactor.y > 0.0 && value.y === 0.0) || (this._imageBasedLightingFactor.y === 0.0 && value.y > 0.0);
        Cesium.Cartesian2.clone(value, this._imageBasedLightingFactor);
    }

    get lightColor(){
        return this._lightColor;
    }

    set lightColor(value){
        let lightColor = this._lightColor;
        if (value === lightColor || Cartesian3.equals(value, lightColor)) {
            return;
        }
        this._shouldRegenerateShaders = this._shouldRegenerateShaders || (Cesium.defined(lightColor) && !Cesium.defined(value)) || (Cesium.defined(value) && !Cesium.defined(lightColor));
        this._lightColor = Cesium.Cartesian3.clone(value, lightColor);
    }

    get luminanceAtZenith(){
         return this._luminanceAtZenith;
    }

    set luminanceAtZenith(value){
        let lum = this._luminanceAtZenith;
        if (value === lum) {
            return;
        }
        this._shouldRegenerateShaders = this._shouldRegenerateShaders || (Cesium.defined(lum) && !Cesium.defined(value)) || (Cesium.defined(value) && !Cesium.defined(lum));
        this._luminanceAtZenith = value;
    }

    get sphericalHarmonicCoefficients(){
         return this._sphericalHarmonicCoefficients;
    }

    set sphericalHarmonicCoefficients(value){
        if (Cesium.defined(value) && (!Cesium.isArray(value) || value.length !== 9)) {
            throw new Cesium.DeveloperError('sphericalHarmonicCoefficients must be an array of 9 Cartesian3 values.');
        }
        //>>includeEnd('debug');
        if (value === this._sphericalHarmonicCoefficients) {
            return;
        }
        this._sphericalHarmonicCoefficients = value;
        this._shouldRegenerateShaders = true;
    }

    get specularEnvironmentMaps(){
          return this._specularEnvironmentMaps;
    }

    set specularEnvironmentMaps(value){
        this._shouldUpdateSpecularMapAtlas = this._shouldUpdateSpecularMapAtlas || value !== this._specularEnvironmentMaps;
        this._specularEnvironmentMaps = value;
    }
    
}
     
    function silhouetteSupported(context) {
        return context.stencilBuffer;
    }

    function isColorShadingEnabled(model) {
        return !Cesium.Color.equals(model.color, Cesium.Color.WHITE) || (model.colorBlendMode !== Cesium.ColorBlendMode.HIGHLIGHT);
    }

    function isClippingEnabled(model) {
        let clippingPlanes = model._clippingPlanes;
        return Cesium.defined(clippingPlanes) && clippingPlanes.enabled && clippingPlanes.length !== 0;
    }

    /**
     * Determines if silhouettes are supported.
     *
     * @param {Scene} scene The scene.
     * @returns {Boolean} <code>true</code> if silhouettes are supported; otherwise, returns <code>false</code>
     */
    M3DModelParser.prototype.silhouetteSupported = function(scene) {
        return silhouetteSupported(scene.context);
    };

    function containsGltfMagic(uint8Array) {
        let magic = Cesium.getMagic(uint8Array);
        return magic === 'glTF';
    }

      /**
     * For the unit tests to verify model caching.
     *
     * @private
     */
    M3DModelParser._gltfCache = gltfCache;

    function getRuntime(model, runtimeName, name) {
        //>>includeStart('debug', pragmas.debug);
        if (model._state !== ModelState.LOADED) {
            throw new Cesium.DeveloperError('The model is not loaded.  Use Model.readyPromise or wait for Model.ready to be true.');
        }

        if (!Cesium.defined(name)) {
            throw new Cesium.DeveloperError('name is required.');
        }
        //>>includeEnd('debug');

        return (model._runtime[runtimeName])[name];
    }


    M3DModelParser.prototype.getNode = function(name) {
        let node = getRuntime(this, 'nodesByName', name);
        return Cesium.defined(node) ? node.publicNode : undefined;
    };

    /**
     * Returns the glTF mesh with the given <code>name</code> property.
     *
     * @param {String} name The glTF name of the mesh.
     *
     * @returns {ModelMesh} The mesh or <code>undefined</code> if no mesh with <code>name</code> exists.
     *
     * @exception {DeveloperError} The model is not loaded.  Use Model.readyPromise or wait for Model.ready to be true.
     */
    M3DModelParser.prototype.getMesh = function(name) {
        return getRuntime(this, 'meshesByName', name);
    };

    /**
     * Returns the glTF material with the given <code>name</code> property.
     *
     * @param {String} name The glTF name of the material.
     * @returns {ModelMaterial} The material or <code>undefined</code> if no material with <code>name</code> exists.
     *
     * @exception {DeveloperError} The model is not loaded.  Use Model.readyPromise or wait for Model.ready to be true.
     */
    M3DModelParser.prototype.getMaterial = function(name) {
        return getRuntime(this, 'materialsByName', name);
    };

    /**
     * Sets the current value of an articulation stage.  After setting one or multiple stage values, call
     * Model.applyArticulations() to cause the node matrices to be recalculated.
     *
     * @param {String} articulationStageKey The name of the articulation, a space, and the name of the stage.
     * @param {Number} value The numeric value of this stage of the articulation.
     *
     * @exception {DeveloperError} The model is not loaded.  Use Model.readyPromise or wait for Model.ready to be true.
     *
     * @see Model#applyArticulations
     */
    M3DModelParser.prototype.setArticulationStage = function(articulationStageKey, value) {
        //>>includeStart('debug', pragmas.debug);
        Check.typeOf.number('value', value);
        //>>includeEnd('debug');

        let stage = getRuntime(this, 'stagesByKey', articulationStageKey);
        let articulation = getRuntime(this, 'articulationsByStageKey', articulationStageKey);
        if (Cesium.defined(stage) && Cesium.defined(articulation)) {
            value = Cesium.Math.clamp(value, stage.minimumValue, stage.maximumValue);
            if (!Cesium.Math.equalsEpsilon(stage.currentValue, value, articulationEpsilon)) {
                stage.currentValue = value;
                articulation.isDirty = true;
            }
        }
    };

    let scratchArticulationCartesian = new Cesium.Cartesian3();
    let scratchArticulationRotation = new Cesium.Matrix3();

    /**
     * Modifies a Matrix4 by applying a transformation for a given value of a stage.  Note this is different usage
     * from the typical <code>result</code> parameter, in that the incoming value of <code>result</code> is
     * meaningful.  Various stages of an articulation can be multiplied together, so their
     * transformations are all merged into a composite Matrix4 representing them all.
     *
     * @param {object} stage The stage of an articulation that is being evaluated.
     * @param {Matrix4} result The matrix to be modified.
     * @returns {Matrix4} A matrix transformed as requested by the articulation stage.
     *
     * @private
     */
    function applyArticulationStageMatrix(stage, result) {
        //>>includeStart('debug', pragmas.debug);
        Check.typeOf.object('stage', stage);
        Check.typeOf.object('result', result);
        //>>includeEnd('debug');

        let value = stage.currentValue;
        let cartesian = scratchArticulationCartesian;
        let rotation;
        switch (stage.type) {
            case 'xRotate':
                rotation = Cesium.Matrix3.fromRotationX(Cesium.Math.toRadians(value), scratchArticulationRotation);
                Cesium.Matrix4.multiplyByMatrix3(result, rotation, result);
                break;
            case 'yRotate':
                rotation = Cesium.Matrix3.fromRotationY(Cesium.Math.toRadians(value), scratchArticulationRotation);
                Cesium.Matrix4.multiplyByMatrix3(result, rotation, result);
                break;
            case 'zRotate':
                rotation = Cesium.Matrix3.fromRotationZ(Cesium.Math.toRadians(value), scratchArticulationRotation);
                Cesium.Matrix4.multiplyByMatrix3(result, rotation, result);
                break;
            case 'xTranslate':
                cartesian.x = value;
                cartesian.y = 0.0;
                cartesian.z = 0.0;
                Cesium.Matrix4.multiplyByTranslation(result, cartesian, result);
                break;
            case 'yTranslate':
                cartesian.x = 0.0;
                cartesian.y = value;
                cartesian.z = 0.0;
                Cesium.Matrix4.multiplyByTranslation(result, cartesian, result);
                break;
            case 'zTranslate':
                cartesian.x = 0.0;
                cartesian.y = 0.0;
                cartesian.z = value;
                Cesium.Matrix4.multiplyByTranslation(result, cartesian, result);
                break;
            case 'xScale':
                cartesian.x = value;
                cartesian.y = 1.0;
                cartesian.z = 1.0;
                Cesium.Matrix4.multiplyByScale(result, cartesian, result);
                break;
            case 'yScale':
                cartesian.x = 1.0;
                cartesian.y = value;
                cartesian.z = 1.0;
                Cesium.Matrix4.multiplyByScale(result, cartesian, result);
                break;
            case 'zScale':
                cartesian.x = 1.0;
                cartesian.y = 1.0;
                cartesian.z = value;
                Cesium.Matrix4.multiplyByScale(result, cartesian, result);
                break;
            case 'uniformScale':
                Cesium.Matrix4.multiplyByUniformScale(result, value, result);
                break;
            default:
                break;
        }
        return result;
    }

    let scratchApplyArticulationTransform = new Cesium.Matrix4();

    /**
     * Applies any modified articulation stages to the matrix of each node that participates
     * in any articulation.  Note that this will overwrite any nodeTransformations on participating nodes.
     *
     * @exception {DeveloperError} The model is not loaded.  Use Model.readyPromise or wait for Model.ready to be true.
     */
    M3DModelParser.prototype.applyArticulations = function() {
        let articulationsByName = this._runtime.articulationsByName;
        for (let articulationName in articulationsByName) {
            if (articulationsByName.hasOwnProperty(articulationName)) {
                let articulation = articulationsByName[articulationName];
                if (articulation.isDirty) {
                    articulation.isDirty = false;
                    let numNodes = articulation.nodes.length;
                    for (let n = 0; n < numNodes; ++n) {
                        let node = articulation.nodes[n];
                        let transform = Cesium.Matrix4.clone(node.originalMatrix, scratchApplyArticulationTransform);

                        let numStages = articulation.stages.length;
                        for (let s = 0; s < numStages; ++s) {
                            let stage = articulation.stages[s];
                            transform = applyArticulationStageMatrix(stage, transform);
                        }
                        node.matrix = transform;
                    }
                }
            }
        }
    };

    ///////////////////////////////////////////////////////////////////////////

    function addBuffersToLoadResources(model) {
        let gltf = model.gltf;
        let loadResources = model._loadResources;
        Cesium.ForEach.buffer(gltf, function(buffer, id) {
            loadResources.buffers[id] = buffer.extras._pipeline.source;
        });
    }

    function bufferLoad(model, id) {
        return function(arrayBuffer) {
            let loadResources = model._loadResources;
            let buffer = new Uint8Array(arrayBuffer);
            --loadResources.pendingBufferLoads;
            model.gltf.buffers[id].extras._pipeline.source = buffer;
        };
    }

    function parseBufferViews(model) {
        let bufferViews = model.gltf.bufferViews;
        let vertexBuffersToCreate = model._loadResources.vertexBuffersToCreate;

        // Only ARRAY_BUFFER here.  ELEMENT_ARRAY_BUFFER created below.
        Cesium.ForEach.bufferView(model.gltf, function(bufferView, id) {
            if (bufferView.target === Cesium.WebGLConstants.ARRAY_BUFFER) {
                vertexBuffersToCreate.enqueue(id);
            }
        });

        let indexBuffersToCreate = model._loadResources.indexBuffersToCreate;
        let indexBufferIds = {};

        // The Cesium Renderer requires knowing the datatype for an index buffer
        // at creation type, which is not part of the glTF bufferview so loop
        // through glTF accessors to create the bufferview's index buffer.
        Cesium.ForEach.accessor(model.gltf, function(accessor) {
            let bufferViewId = accessor.bufferView;
            if (!Cesium.defined(bufferViewId)) {
                return;
            }

            let bufferView = bufferViews[bufferViewId];
            if ((bufferView.target === Cesium.WebGLConstants.ELEMENT_ARRAY_BUFFER) && !Cesium.defined(indexBufferIds[bufferViewId])) {
                indexBufferIds[bufferViewId] = true;
                indexBuffersToCreate.enqueue({
                    id : bufferViewId,
                    componentType : accessor.componentType
                });
            }
        });
    }

    function parseTechniques(model) {
        // retain references to gltf techniques
        let gltf = model.gltf;
        if (!Cesium.hasExtension(gltf, 'KHR_techniques_webgl')) {
            return;
        }

        let sourcePrograms = model._sourcePrograms;
        let sourceTechniques = model._sourceTechniques;
        let programs = gltf.extensions.KHR_techniques_webgl.programs;

        Cesium.ForEach.technique(gltf, function(technique, techniqueId) {
            sourceTechniques[techniqueId] = Cesium.clone(technique);

            let programId = technique.program;
            if (!Cesium.defined(sourcePrograms[programId])) {
                sourcePrograms[programId] = Cesium.clone(programs[programId]);
            }
        });
    }

    function shaderLoad(model, type, id) {
        return function(source) {
            let loadResources = model._loadResources;
            loadResources.shaders[id] = {
                source : source,
                type : type,
                bufferView : undefined
            };
            --loadResources.pendingShaderLoads;
            model._rendererResources.sourceShaders[id] = source;
        };
    }

    function parseShaders(model) {
        let gltf = model.gltf;
        let buffers = gltf.buffers;
        let bufferViews = gltf.bufferViews;
        let sourceShaders = model._rendererResources.sourceShaders;
        Cesium.ForEach.shader(gltf, function(shader, id) {
            // Shader references either uri (external or base64-encoded) or bufferView
            if (Cesium.defined(shader.bufferView)) {
                let bufferViewId = shader.bufferView;
                let bufferView = bufferViews[bufferViewId];
                let bufferId = bufferView.buffer;
                let buffer = buffers[bufferId];
                let source = Cesium.getStringFromTypedArray(buffer.extras._pipeline.source, bufferView.byteOffset, bufferView.byteLength);
                sourceShaders[id] = source;
            } else if (Cesium.defined(shader.extras._pipeline.source)) {
                sourceShaders[id] = shader.extras._pipeline.source;
            } else {
                ++model._loadResources.pendingShaderLoads;

                let shaderResource = model._resource.getDerivedResource({
                    url: shader.uri
                });

                shaderResource.fetchText()
                    .then(shaderLoad(model, shader.type, id))
                    .otherwise(ModelUtility.getFailedLoadFunction(model, 'shader', shaderResource.url));
            }
        });
    }

    function parsePrograms(model) {
        let sourceTechniques = model._sourceTechniques;
        for (let techniqueId in sourceTechniques) {
            if (sourceTechniques.hasOwnProperty(techniqueId)) {
                let technique = sourceTechniques[techniqueId];
                model._loadResources.programsToCreate.enqueue({
                    programId: technique.program,
                    techniqueId: techniqueId
                });
            }
        }
    }

    function parseArticulations(model) {
        let articulationsByName = {};
        let articulationsByStageKey = {};
        let runtimeStagesByKey = {};

        model._runtime.articulationsByName = articulationsByName;
        model._runtime.articulationsByStageKey = articulationsByStageKey;
        model._runtime.stagesByKey = runtimeStagesByKey;

        let gltf = model.gltf;
        if (!Cesium.hasExtension(gltf, 'AGI_articulations') || !Cesium.defined(gltf.extensions) || !Cesium.defined(gltf.extensions.AGI_articulations)) {
            return;
        }

        let gltfArticulations = gltf.extensions.AGI_articulations.articulations;
        if (!Cesium.defined(gltfArticulations)) {
            return;
        }

        let numArticulations = gltfArticulations.length;
        for (let i = 0; i < numArticulations; ++i) {
            let articulation = Cesium.clone(gltfArticulations[i]);
            articulation.nodes = [];
            articulation.isDirty = true;
            articulationsByName[articulation.name] = articulation;

            let numStages = articulation.stages.length;
            for (let s = 0; s < numStages; ++s) {
                let stage = articulation.stages[s];
                stage.currentValue = stage.initialValue;

                let stageKey = articulation.name + ' ' + stage.name;
                articulationsByStageKey[stageKey] = articulation;
                runtimeStagesByKey[stageKey] = stage;
            }
        }
    }

    function imageLoad(model, textureId) {
        return function(image) {
            let loadResources = model._loadResources;
            --loadResources.pendingTextureLoads;
            loadResources.texturesToCreate.enqueue({
                id : textureId,
                image : image,
                bufferView : image.bufferView,
                width : image.width,
                height : image.height,
                internalFormat : image.internalFormat
            });
        };
    }

    let ktxRegex = /(^data:image\/ktx)|(\.ktx$)/i;
    let crnRegex = /(^data:image\/crn)|(\.crn$)/i;

    function parseTextures(model, context, supportsWebP) {
        let gltf = model.gltf;
        let images = gltf.images;
        let uri;
        Cesium.ForEach.texture(gltf, function(texture, id) {
            let imageId = texture.source;

            if (Cesium.defined(texture.extensions) && Cesium.defined(texture.extensions.EXT_texture_webp) && supportsWebP) {
                imageId = texture.extensions.EXT_texture_webp.source;
            }

            let gltfImage = images[imageId];
            let extras = gltfImage.extras;

            let bufferViewId = gltfImage.bufferView;
            let mimeType = gltfImage.mimeType;
            //hys
            if(mimeType === ''){
                mimeType = 'image/vnd-ms.dds';
                ///mimeType = 'image/png';
            }
            uri = gltfImage.uri;

            // First check for a compressed texture
            if (Cesium.defined(extras) && Cesium.defined(extras.compressedImage3DTiles)) {
                let crunch = extras.compressedImage3DTiles.crunch;
                let s3tc = extras.compressedImage3DTiles.s3tc;
                let pvrtc = extras.compressedImage3DTiles.pvrtc1;
                let etc1 = extras.compressedImage3DTiles.etc1;

                if (context.s3tc && Cesium.defined(crunch)) {
                    mimeType = crunch.mimeType;
                    if (Cesium.defined(crunch.bufferView)) {
                        bufferViewId = crunch.bufferView;
                    } else {
                        uri = crunch.uri;
                    }
                } else if (context.s3tc && Cesium.defined(s3tc)) {
                    mimeType = s3tc.mimeType;
                    if (Cesium.defined(s3tc.bufferView)) {
                        bufferViewId = s3tc.bufferView;
                    } else {
                        uri = s3tc.uri;
                    }
                } else if (context.pvrtc && Cesium.defined(pvrtc)) {
                    mimeType = pvrtc.mimeType;
                    if (Cesium.defined(pvrtc.bufferView)) {
                        bufferViewId = pvrtc.bufferView;
                    } else {
                        uri = pvrtc.uri;
                    }
                } else if (context.etc1 && Cesium.defined(etc1)) {
                    mimeType = etc1.mimeType;
                    if (Cesium.defined(etc1.bufferView)) {
                        bufferViewId = etc1.bufferView;
                    } else {
                        uri = etc1.uri;
                    }
                }
            }

            // Image references either uri (external or base64-encoded) or bufferView
            if (Cesium.defined(bufferViewId)) {

                model._loadResources.texturesToCreateFromBufferView.enqueue({
                    id : id,
                    image : undefined,
                    bufferView : bufferViewId,
                    mimeType : mimeType
                });
            } else {
                ++model._loadResources.pendingTextureLoads;

                let imageResource = model._resource.getDerivedResource({
                    url : uri
                });

                let promise;
                if (ktxRegex.test(uri)) {
                    promise = loadKTX(imageResource);
                } else if (crnRegex.test(uri)) {
                    promise = loadCRN(imageResource);
                } else {
                    promise = imageResource.fetchImage();
                }
                promise.then(imageLoad(model, id, imageId)).otherwise(Cesium.ModelUtility.getFailedLoadFunction(model, 'image', imageResource.url));
            }
        });
    }

    let scratchArticulationStageInitialTransform = new Cesium.Matrix4();

    function parseNodes(model) {
        let runtimeNodes = {};
        let runtimeNodesByName = {};
        let skinnedNodes = [];

        let skinnedNodesIds = model._loadResources.skinnedNodesIds;
        let articulationsByName = model._runtime.articulationsByName;

        Cesium.ForEach.node(model.gltf, function(node, id) {
            let runtimeNode = {
                // Animation targets
                matrix : undefined,
                translation : undefined,
                rotation : undefined,
                scale : undefined,

                // Per-node show inherited from parent
                computedShow : true,

                // Computed transforms
                transformToRoot : new Cesium.Matrix4(),
                computedMatrix : new Cesium.Matrix4(),
                dirtyNumber : 0,                    // The frame this node was made dirty by an animation; for graph traversal

                // Rendering
                commands : [],                      // empty for transform, light, and camera nodes

                // Skinned node
                inverseBindMatrices : undefined,    // undefined when node is not skinned
                bindShapeMatrix : undefined,        // undefined when node is not skinned or identity
                joints : [],                        // empty when node is not skinned
                computedJointMatrices : [],         // empty when node is not skinned

                // Joint node
                jointName : node.jointName,         // undefined when node is not a joint

                weights : [],

                // Graph pointers
                children : [],                      // empty for leaf nodes
                parents : [],                       // empty for root nodes

                // Publicly-accessible ModelNode instance to modify animation targets
                publicNode : undefined
            };
            runtimeNode.publicNode = new Cesium.ModelNode(model, node, runtimeNode, id, Cesium.ModelUtility.getTransform(node));

            runtimeNodes[id] = runtimeNode;
            runtimeNodesByName[node.name] = runtimeNode;

            if (Cesium.defined(node.skin)) {
                skinnedNodesIds.push(id);
                skinnedNodes.push(runtimeNode);
            }

            if (Cesium.defined(node.extensions) && Cesium.defined(node.extensions.AGI_articulations)) {
                let articulationName = node.extensions.AGI_articulations.articulationName;
                if (Cesium.defined(articulationName)) {
                    let transform = Cesium.Matrix4.clone(runtimeNode.publicNode.originalMatrix, scratchArticulationStageInitialTransform);
                    let articulation = articulationsByName[articulationName];
                    articulation.nodes.push(runtimeNode.publicNode);

                    let numStages = articulation.stages.length;
                    for (let s = 0; s < numStages; ++s) {
                        let stage = articulation.stages[s];
                        transform = applyArticulationStageMatrix(stage, transform);
                    }
                    runtimeNode.publicNode.matrix = transform;
                }
            }
        });

        model._runtime.nodes = runtimeNodes;
        model._runtime.nodesByName = runtimeNodesByName;
        model._runtime.skinnedNodes = skinnedNodes;
    }

    function parseMaterials(model) {
        let gltf = model.gltf;
        let techniques = model._sourceTechniques;

        let runtimeMaterialsByName = {};
        let runtimeMaterialsById = {};
        let uniformMaps = model._uniformMaps;

        Cesium.ForEach.material(gltf, function(material, materialId) {
            // Allocated now so ModelMaterial can keep a reference to it.
            uniformMaps[materialId] = {
                uniformMap : undefined,
                values : undefined,
                jointMatrixUniformName : undefined,
                morphWeightsUniformName : undefined
            };

            let modelMaterial = new Cesium.ModelMaterial(model, material, materialId);

            if (Cesium.defined(material.extensions) && Cesium.defined(material.extensions.KHR_techniques_webgl)) {
                let techniqueId = material.extensions.KHR_techniques_webgl.technique;
                modelMaterial._technique = techniqueId;
                modelMaterial._program = techniques[techniqueId].program;

                Cesium.ForEach.materialValue(material, function(value, uniformName) {
                    if (!Cesium.defined(modelMaterial._values)) {
                        modelMaterial._values = {};
                    }

                    modelMaterial._values[uniformName] = Cesium.clone(value);
                });
            }

            runtimeMaterialsByName[material.name] = modelMaterial;
            runtimeMaterialsById[materialId] = modelMaterial;
        });

        model._runtime.materialsByName = runtimeMaterialsByName;
        model._runtime.materialsById = runtimeMaterialsById;
    }

    function parseMeshes(model) {
        let runtimeMeshesByName = {};
        let runtimeMaterialsById = model._runtime.materialsById;

        Cesium.ForEach.mesh(model.gltf, function(mesh, meshId) {
            runtimeMeshesByName[mesh.name] = new Cesium.ModelMesh(mesh, runtimeMaterialsById, meshId);
            if (Cesium.defined(model.extensionsUsed.WEB3D_quantized_attributes) || model._dequantizeInShader) {
                // Cache primitives according to their program
                Cesium.ForEach.meshPrimitive(mesh, function(primitive, primitiveId) {
                    let programId = getProgramForPrimitive(model, primitive);
                    let programPrimitives = model._programPrimitives[programId];
                    if (!Cesium.defined(programPrimitives)) {
                        programPrimitives = {};
                        model._programPrimitives[programId] = programPrimitives;
                    }
                    programPrimitives[meshId + '.primitive.' + primitiveId] = primitive;
                });
            }
        });

        model._runtime.meshesByName = runtimeMeshesByName;
    }

    let scratchVertexBufferJob = new CreateVertexBufferJob();
    let scratchIndexBufferJob = new CreateIndexBufferJob();
    
    function createBuffers(model, frameState) {
        let loadResources = model._loadResources;

        if (loadResources.pendingBufferLoads !== 0) {
            return;
        }

        let context = frameState.context;
        let vertexBuffersToCreate = loadResources.vertexBuffersToCreate;
        let indexBuffersToCreate = loadResources.indexBuffersToCreate;
        let i;

        if (model.asynchronous) {
            while (vertexBuffersToCreate.length > 0) {
                scratchVertexBufferJob.set(vertexBuffersToCreate.peek(), model, context);
                if (!frameState.jobScheduler.execute(scratchVertexBufferJob, Cesium.JobType.BUFFER)) {
                    break;
                }
                vertexBuffersToCreate.dequeue();
            }

            while (indexBuffersToCreate.length > 0) {
                i = indexBuffersToCreate.peek();
                scratchIndexBufferJob.set(i.id, i.componentType, model, context);
                if (!frameState.jobScheduler.execute(scratchIndexBufferJob, Cesium.JobType.BUFFER)) {
                    break;
                }
                indexBuffersToCreate.dequeue();
            }7
        } else {
            while (vertexBuffersToCreate.length > 0) {
                CreateVertexBufferJob.createVertexBuffer(vertexBuffersToCreate.dequeue(), model, context);
            }

            while (indexBuffersToCreate.length > 0) {
                i = indexBuffersToCreate.dequeue();
                CreateIndexBufferJob.createIndexBuffer(i.id, i.componentType, model, context);
            }
        }
    }

    function getProgramForPrimitive(model, primitive) {
        let material = model._runtime.materialsById[primitive.material];
        if (!Cesium.defined(material)) {
            return;
        }

        return material._program;
    }

    function modifyShaderForQuantizedAttributes(shader, programName, model) {
        let primitive;
        let primitives = model._programPrimitives[programName];

        // If no primitives were cached for this program, there's no need to modify the shader
        if (!Cesium.defined(primitives)) {
            return shader;
        }

        let primitiveId;
        for (primitiveId in primitives) {
            if (primitives.hasOwnProperty(primitiveId)) {
                primitive = primitives[primitiveId];
                if (getProgramForPrimitive(model, primitive) === programName) {
                    break;
                }
            }
        }

        // This is not needed after the program is processed, free the memory
        model._programPrimitives[programName] = undefined;

        let result;
        if (model.extensionsUsed.WEB3D_quantized_attributes) {
            result = Cesium.ModelUtility.modifyShaderForQuantizedAttributes(model.gltf, primitive, shader);
            model._quantizedUniforms[programName] = result.uniforms;
        } else {
            let decodedData = model._decodedData[primitiveId];
            if (Cesium.defined(decodedData)) {
                result = Cesium.ModelUtility.modifyShaderForDracoQuantizedAttributes(model.gltf, primitive, shader, decodedData.attributes);
            } else {
                return shader;
            }
        }

        return result.shader;
    }

    function modifyShaderForColor(shader) {
        shader = Cesium.ShaderSource.replaceMain(shader, 'gltf_blend_main');
        shader +=
            'uniform vec4 gltf_color; \n' +
            'uniform float gltf_colorBlend; \n' +
            'void main() \n' +
            '{ \n' +
            '    gltf_blend_main(); \n' +
            '    gl_FragColor.rgb = mix(gl_FragColor.rgb, gltf_color.rgb, gltf_colorBlend); \n' +
            '    float highlight = ceil(gltf_colorBlend); \n' +
            '    gl_FragColor.rgb *= mix(gltf_color.rgb, vec3(1.0), highlight); \n' +
            '    gl_FragColor.a *= gltf_color.a; \n' +
            '} \n';

        return shader;
    }

    function modifyShader(shader, programName, callback) {
        if (Cesium.defined(callback)) {
            shader = callback(shader, programName);
        }
        return shader;
    }
    let CreateProgramJob = function() {
        this.programToCreate = undefined;
        this.model = undefined;
        this.context = undefined;
    };

    CreateProgramJob.prototype.set = function(programToCreate, model, context) {
        this.programToCreate = programToCreate;
        this.model = model;
        this.context = context;
    };

    CreateProgramJob.prototype.execute = function() {
        createProgram(this.programToCreate, this.model, this.context);
    };

    function createProgram(programToCreate, model, context) {
        let programId = programToCreate.programId;
        let techniqueId = programToCreate.techniqueId;
        let program = model._sourcePrograms[programId];
        let shaders = model._rendererResources.sourceShaders;

        let vs = shaders[program.vertexShader];
        let fs = shaders[program.fragmentShader];

        let quantizedVertexShaders = model._quantizedVertexShaders;
        let toClipCoordinatesGLSL = model._toClipCoordinatesGLSL[programId];

        if (model.extensionsUsed.WEB3D_quantized_attributes || model._dequantizeInShader) {
            let quantizedVS = quantizedVertexShaders[programId];
            if (!Cesium.defined(quantizedVS)) {
                quantizedVS = modifyShaderForQuantizedAttributes(vs, programId, model);
                quantizedVertexShaders[programId] = quantizedVS;
            }
            vs = quantizedVS;
        }

        let drawVS = modifyShader(vs, programId, model._vertexShaderLoaded);
        let drawFS = modifyShader(fs, programId, model._fragmentShaderLoaded);

        // Internet Explorer seems to have problems with discard (for clipping planes) after too many levels of indirection:
        // https://github.com/AnalyticalGraphicsInc/cesium/issues/6575.
        // For IE log depth code is defined out anyway due to unsupported WebGL extensions, so the wrappers can be omitted.
        if (!Cesium.FeatureDetection.isInternetExplorer()) {
            drawVS = Cesium.ModelUtility.modifyVertexShaderForLogDepth(drawVS, toClipCoordinatesGLSL);
            drawFS = Cesium.ModelUtility.modifyFragmentShaderForLogDepth(drawFS);
        }
        //fgy这些可以去掉
        // if (!Cesium.defined(model._uniformMapLoaded)) {
        //     drawFS = 'uniform vec4 czm_pickColor;\n' + drawFS;
        // }

        let useIBL = model._imageBasedLightingFactor.x > 0.0 || model._imageBasedLightingFactor.y > 0.0;
        if (useIBL) {
            drawFS = '#define USE_IBL_LIGHTING \n\n' + drawFS;
        }

        // if (Cesium.defined(model._lightColor)) {
        //     drawFS = '#define USE_CUSTOM_LIGHT_COLOR \n\n' + drawFS;
        // }

        if (model._sourceVersion !== '2.0' || model._sourceKHRTechniquesWebGL) {
            drawFS = Cesium.ShaderSource.replaceMain(drawFS, 'non_gamma_corrected_main');
            drawFS =
                drawFS +
                '\n' +
                'void main() { \n' +
                '    non_gamma_corrected_main(); \n' +
                '    gl_FragColor = czm_gammaCorrect(gl_FragColor); \n' +
                '} \n';
        }

        if (Cesium.OctahedralProjectedCubeMap.isSupported(context)) {
            let usesSH = Cesium.defined(model._sphericalHarmonicCoefficients) || model._useDefaultSphericalHarmonics;
            let usesSM = (Cesium.defined(model._specularEnvironmentMapAtlas) && model._specularEnvironmentMapAtlas.ready) || model._useDefaultSpecularMaps;
            let addMatrix = usesSH || usesSM || useIBL;
            if (addMatrix) {
                drawFS = 'uniform mat4 gltf_clippingPlanesMatrix; \n' + drawFS;
            }

            if (Cesium.defined(model._sphericalHarmonicCoefficients)) {
                drawFS = '#define DIFFUSE_IBL \n' + '#define CUSTOM_SPHERICAL_HARMONICS \n' + 'uniform vec3 gltf_sphericalHarmonicCoefficients[9]; \n' + drawFS;
            } else if (model._useDefaultSphericalHarmonics) {
                drawFS = '#define DIFFUSE_IBL \n' + drawFS;
            }

            if (Cesium.defined(model._specularEnvironmentMapAtlas) && model._specularEnvironmentMapAtlas.ready) {
                drawFS = '#define SPECULAR_IBL \n' + '#define CUSTOM_SPECULAR_IBL \n' + 'uniform sampler2D gltf_specularMap; \n' + 'uniform vec2 gltf_specularMapSize; \n' + 'uniform float gltf_maxSpecularLOD; \n' + drawFS;
            } else if (model._useDefaultSpecularMaps) {
                drawFS = '#define SPECULAR_IBL \n' + drawFS;
            }
        }

        if (Cesium.defined(model._luminanceAtZenith)) {
            drawFS = '#define USE_SUN_LUMINANCE \n' + 'uniform float gltf_luminanceAtZenith;\n' + drawFS;
        }

        createAttributesAndProgram(programId, techniqueId, drawFS, drawVS, model, context);
    }

    function recreateProgram(programToCreate, model, context) {
        let programId = programToCreate.programId;
        let techniqueId = programToCreate.techniqueId;
        let program = model._sourcePrograms[programId];
        let shaders = model._rendererResources.sourceShaders;

        let quantizedVertexShaders = model._quantizedVertexShaders;
        let toClipCoordinatesGLSL = model._toClipCoordinatesGLSL[programId];

        let clippingPlaneCollection = model.clippingPlanes;
        let addClippingPlaneCode = false;//isClippingEnabled(model);

        let vs = shaders[program.vertexShader];
        let fs = shaders[program.fragmentShader];

        if (model.extensionsUsed.WEB3D_quantized_attributes || model._dequantizeInShader) {
            vs = quantizedVertexShaders[programId];
        }

        let finalFS = fs;
        // if (isColorShadingEnabled(model)) {
        //     finalFS = Model._modifyShaderForColor(finalFS);
        // }
        // if (addClippingPlaneCode) {
        //     finalFS = modifyShaderForClippingPlanes(finalFS, clippingPlaneCollection, context);
        // }

        let drawVS = modifyShader(vs, programId, model._vertexShaderLoaded);
        let drawFS = modifyShader(finalFS, programId, model._fragmentShaderLoaded);

        if (!Cesium.FeatureDetection.isInternetExplorer()) {
            drawVS = Cesium.ModelUtility.modifyVertexShaderForLogDepth(drawVS, toClipCoordinatesGLSL);
            drawFS = Cesium.ModelUtility.modifyFragmentShaderForLogDepth(drawFS);
        }

        if (!Cesium.defined(model._uniformMapLoaded)) {
            drawFS = 'uniform vec4 czm_pickColor;\n' + drawFS;
        }

        let useIBL = model._imageBasedLightingFactor.x > 0.0 || model._imageBasedLightingFactor.y > 0.0;
        if (useIBL) {
            drawFS = '#define USE_IBL_LIGHTING \n\n' + drawFS;
        }

        // if (Cesium.defined(model._lightColor)) {
        //     drawFS = '#define USE_CUSTOM_LIGHT_COLOR \n\n' + drawFS;
        // }

        if (model._sourceVersion !== '2.0' || model._sourceKHRTechniquesWebGL) {
            drawFS = Cesium.ShaderSource.replaceMain(drawFS, 'non_gamma_corrected_main');
            drawFS =
                drawFS +
                '\n' +
                'void main() { \n' +
                '    non_gamma_corrected_main(); \n' +
                '    gl_FragColor = czm_gammaCorrect(gl_FragColor); \n' +
                '} \n';
        }

        if (Cesium.OctahedralProjectedCubeMap.isSupported(context)) {
            let usesSH = Cesium.defined(model._sphericalHarmonicCoefficients) || model._useDefaultSphericalHarmonics;
            let usesSM = (Cesium.defined(model._specularEnvironmentMapAtlas) && model._specularEnvironmentMapAtlas.ready) || model._useDefaultSpecularMaps;
            let addMatrix = !addClippingPlaneCode && (usesSH || usesSM || useIBL);
            if (addMatrix) {
                drawFS = 'uniform mat4 gltf_clippingPlanesMatrix; \n' + drawFS;
            }

            if (Cesium.defined(model._sphericalHarmonicCoefficients)) {
                drawFS = '#define DIFFUSE_IBL \n' + '#define CUSTOM_SPHERICAL_HARMONICS \n' + 'uniform vec3 gltf_sphericalHarmonicCoefficients[9]; \n' + drawFS;
            } else if (model._useDefaultSphericalHarmonics) {
                drawFS = '#define DIFFUSE_IBL \n' + drawFS;
            }

            if (Cesium.defined(model._specularEnvironmentMapAtlas) && model._specularEnvironmentMapAtlas.ready) {
                drawFS = '#define SPECULAR_IBL \n' + '#define CUSTOM_SPECULAR_IBL \n' + 'uniform sampler2D gltf_specularMap; \n' + 'uniform vec2 gltf_specularMapSize; \n' + 'uniform float gltf_maxSpecularLOD; \n' + drawFS;
            } else if (model._useDefaultSpecularMaps) {
                drawFS = '#define SPECULAR_IBL \n' + drawFS;
            }
        }

        if (Cesium.defined(model._luminanceAtZenith)) {
            drawFS = '#define USE_SUN_LUMINANCE \n' + 'uniform float gltf_luminanceAtZenith;\n' + drawFS;
        }

        createAttributesAndProgram(programId, techniqueId, drawFS, drawVS, model, context);
    }

    function createAttributesAndProgram(programId, techniqueId, drawFS, drawVS, model, context) {
        let technique = model._sourceTechniques[techniqueId];
        let attributeLocations = Cesium.ModelUtility.createAttributeLocations(technique, model._precreatedAttributes);

        model._rendererResources.programs[programId] = Cesium.ShaderProgram.fromCache({
            context : context,
            vertexShaderSource : drawVS,
            fragmentShaderSource : drawFS,
            attributeLocations : attributeLocations
        });
    }

    let scratchCreateProgramJob = new CreateProgramJob();

    function createPrograms(model, frameState) {
        let loadResources = model._loadResources;
        let programsToCreate = loadResources.programsToCreate;

        if (loadResources.pendingShaderLoads !== 0) {
            return;
        }

        // PERFORMANCE_IDEA: this could be more fine-grained by looking
        // at the shader's bufferView's to determine the buffer dependencies.
        if (loadResources.pendingBufferLoads !== 0) {
            return;
        }

        let context = frameState.context;

        if (model.asynchronous) {
            while (programsToCreate.length > 0) {
                scratchCreateProgramJob.set(programsToCreate.peek(), model, context);
                if (!frameState.jobScheduler.execute(scratchCreateProgramJob, Cesium.JobType.PROGRAM)) {
                    break;
                }
                programsToCreate.dequeue();
            }
        } else {
            // Create all loaded programs this frame
            while (programsToCreate.length > 0) {
                createProgram(programsToCreate.dequeue(), model, context);
            }
        }
    }

    function getOnImageCreatedFromTypedArray(loadResources, gltfTexture) {
        return function(image) {
            loadResources.texturesToCreate.enqueue({
                id : gltfTexture.id,
                image : image,
                bufferView : undefined
            });

            --loadResources.pendingBufferViewToImage;
        };
    }

    function loadTexturesFromBufferViews(model) {
        let loadResources = model._loadResources;

        if (loadResources.pendingBufferLoads !== 0) {
            return;
        }

        while (loadResources.texturesToCreateFromBufferView.length > 0) {
            let gltfTexture = loadResources.texturesToCreateFromBufferView.dequeue();

            let gltf = model.gltf;
            let bufferView = gltf.bufferViews[gltfTexture.bufferView];
            let imageId = gltf.textures[gltfTexture.id].source;

            let onerror = Cesium.ModelUtility.getFailedLoadFunction(model, 'image', 'id: ' + gltfTexture.id + ', bufferView: ' + gltfTexture.bufferView);

            if (gltfTexture.mimeType === 'image/ktx') {
                loadKTX(loadResources.getBuffer(bufferView)).then(imageLoad(model, gltfTexture.id, imageId)).otherwise(onerror);
                ++model._loadResources.pendingTextureLoads;
            } else if (gltfTexture.mimeType === 'image/crn') {
                loadCRN(loadResources.getBuffer(bufferView)).then(imageLoad(model, gltfTexture.id, imageId)).otherwise(onerror);
                ++model._loadResources.pendingTextureLoads;
            } else {
                let onload = getOnImageCreatedFromTypedArray(loadResources, gltfTexture);
                Cesium.loadImageFromTypedArray({
                    uint8Array: loadResources.getBuffer(bufferView),
                    format: gltfTexture.mimeType,
                    flipY: false
                })
                    .then(onload).otherwise(onerror);
                ++loadResources.pendingBufferViewToImage;
            }
        }
    }

    function createSamplers(model) {
        let loadResources = model._loadResources;
        if (loadResources.createSamplers) {
            loadResources.createSamplers = false;

            let rendererSamplers = model._rendererResources.samplers;
            Cesium.ForEach.sampler(model.gltf, function(sampler, samplerId) {
                rendererSamplers[samplerId] = new Cesium.Sampler({
                    wrapS: sampler.wrapS,
                    wrapT: sampler.wrapT,
                    minificationFilter: sampler.minFilter,
                    magnificationFilter: sampler.magFilter
                });
            });
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    class CreateTextureJob{
        constructor(){
            this.gltfTexture = undefined;
            this.model = undefined;
            this.context = undefined;
        }
    };

    CreateTextureJob.prototype.set = function(gltfTexture, model, context) {
        this.gltfTexture = gltfTexture;
        this.model = model;
        this.context = context;
    };

    CreateTextureJob.prototype.execute = function() {
        createTexture(this.gltfTexture, this.model, this.context);
    };

    ///////////////////////////////////////////////////////////////////////////
    function createTexture(gltfTexture, model, context) {
        let textures = model.gltf.textures;
        let texture = textures[gltfTexture.id];

        let rendererSamplers = model._rendererResources.samplers;
        let sampler = rendererSamplers[texture.sampler];
        if (!Cesium.defined(sampler)) {
            sampler = new Cesium.Sampler({
                wrapS : Cesium.TextureWrap.REPEAT,
                wrapT : Cesium.TextureWrap.REPEAT
            });
        }

        let usesTextureTransform = false;
        let materials = model.gltf.materials;
        let materialsLength = materials.length;
        for (let i = 0; i < materialsLength; ++i) {
            let material = materials[i];
            if (Cesium.defined(material.extensions) && Cesium.defined(material.extensions.KHR_techniques_webgl)) {
                let values = material.extensions.KHR_techniques_webgl.values;
                for (let valueName in values) {
                    if (values.hasOwnProperty(valueName) && valueName.indexOf('Texture') !== -1) {
                        let value = values[valueName];
                        if (value.index === gltfTexture.id && Cesium.defined(value.extensions) && Cesium.defined(value.extensions.KHR_texture_transform)) {
                            usesTextureTransform = true;
                            break;
                        }
                    }
                }
            }
            if (usesTextureTransform) {
                break;
            }
        }

        let wrapS = sampler.wrapS;
        let wrapT = sampler.wrapT;
        let minFilter = sampler.minificationFilter;

        if (usesTextureTransform && minFilter !== Cesium.TextureMinificationFilter.LINEAR && minFilter !== Cesium.TextureMinificationFilter.NEAREST) {
            if (minFilter === Cesium.TextureMinificationFilter.NEAREST_MIPMAP_NEAREST || minFilter === Cesium.TextureMinificationFilter.NEAREST_MIPMAP_LINEAR) {
                minFilter = Cesium.TextureMinificationFilter.NEAREST;
            } else {
                minFilter = Cesium.TextureMinificationFilter.LINEAR;
            }

            sampler = new Cesium.Sampler({
                wrapS : sampler.wrapS,
                wrapT : sampler.wrapT,
                textureMinificationFilter : minFilter,
                textureMagnificationFilter : sampler.magnificationFilter
            });
        }

        let internalFormat = gltfTexture.internalFormat;

        let mipmap =
            (!(Cesium.defined(internalFormat) && Cesium.PixelFormat.isCompressedFormat(internalFormat))) &&
            ((minFilter === Cesium.TextureMinificationFilter.NEAREST_MIPMAP_NEAREST) ||
             (minFilter === Cesium.TextureMinificationFilter.NEAREST_MIPMAP_LINEAR) ||
             (minFilter === Cesium.TextureMinificationFilter.LINEAR_MIPMAP_NEAREST) ||
             (minFilter === Cesium.TextureMinificationFilter.LINEAR_MIPMAP_LINEAR));
        let requiresNpot = mipmap ||
           (wrapS === Cesium.TextureWrap.REPEAT) ||
           (wrapS === Cesium.TextureWrap.MIRRORED_REPEAT) ||
           (wrapT === Cesium.TextureWrap.REPEAT) ||
           (wrapT === Cesium.TextureWrap.MIRRORED_REPEAT);

        let tx;
        let source = gltfTexture.image;

        if (Cesium.defined(internalFormat)) {
            tx = new Cesium.Texture({
                context : context,
                source : {
                    arrayBufferView : gltfTexture.bufferView
                },
                width : gltfTexture.width,
                height : gltfTexture.height,
                pixelFormat : internalFormat,
                sampler : sampler
            });
        } else if (Cesium.defined(source)) {
            let npot = !Cesium.Math.isPowerOfTwo(source.width) || !Cesium.Math.isPowerOfTwo(source.height);

            if (requiresNpot && npot) {
                // WebGL requires power-of-two texture dimensions for mipmapping and REPEAT/MIRRORED_REPEAT wrap modes.
                let canvas = document.createElement('canvas');
                canvas.width = Cesium.Math.nextPowerOfTwo(source.width);
                canvas.height = Cesium.Math.nextPowerOfTwo(source.height);
                let canvasContext = canvas.getContext('2d');
                canvasContext.drawImage(source, 0, 0, source.width, source.height, 0, 0, canvas.width, canvas.height);
                source = canvas;
            }

            tx = new Cesium.Texture({
                context : context,
                source : source,
                pixelFormat : texture.internalFormat,
                pixelDatatype : texture.type,
                sampler : sampler,
                flipY : false
            });
            // GLTF_SPEC: Support TEXTURE_CUBE_MAP.  https://github.com/KhronosGroup/glTF/issues/40
            if (mipmap) {
                tx.generateMipmap();
            }
        }
        if (Cesium.defined(tx)) {
            model._rendererResources.textures[gltfTexture.id] = tx;
            model._texturesByteLength += tx.sizeInBytes;
        }
    }

    function createTextures(model, frameState) {
        let context = frameState.context;
        let texturesToCreate = model._loadResources.texturesToCreate;

        if (model.asynchronous) {
            while (texturesToCreate.length > 0) {
                scratchCreateTextureJob.set(texturesToCreate.peek(), model, context);
                if (!frameState.jobScheduler.execute(scratchCreateTextureJob, Cesium.JobType.TEXTURE)) {
                    break;
                }
                texturesToCreate.dequeue();
            }
        } else {
            // Create all loaded textures this frame
            while (texturesToCreate.length > 0) {
                createTexture(texturesToCreate.dequeue(), model, context);
            }
        }
    }

    function getAttributeLocations(model, primitive) {
        let techniques = model._sourceTechniques;

        // Retrieve the compiled shader program to assign index values to attributes
        let attributeLocations = {};

        let location;
        let index;
        let material = model._runtime.materialsById[primitive.material];
        if (!Cesium.defined(material)) {
            return attributeLocations;
        }

        let technique = techniques[material._technique];
        if (!Cesium.defined(technique)) {
            return attributeLocations;
        }

        let attributes = technique.attributes;
        let program = model._rendererResources.programs[technique.program];
        let programVertexAttributes = program.vertexAttributes;
        let programAttributeLocations = program._attributeLocations;

        // Note: WebGL shader compiler may have optimized and removed some attributes from programVertexAttributes
        for (location in programVertexAttributes) {
            if (programVertexAttributes.hasOwnProperty(location)) {
                let attribute = attributes[location];
                if (Cesium.defined(attribute)) {
                    index = programAttributeLocations[location];
                    attributeLocations[attribute.semantic] = index;
                }
            }
        }

        // Always add pre-created attributes.
        // Some pre-created attributes, like per-instance pickIds, may be compiled out of the draw program
        // but should be included in the list of attribute locations for the pick program.
        // This is safe to do since programVertexAttributes and programAttributeLocations are equivalent except
        // that programVertexAttributes optimizes out unused attributes.
        let precreatedAttributes = model._precreatedAttributes;
        if (Cesium.defined(precreatedAttributes)) {
            for (location in precreatedAttributes) {
                if (precreatedAttributes.hasOwnProperty(location)) {
                    index = programAttributeLocations[location];
                    attributeLocations[location] = index;
                }
            }
        }

        return attributeLocations;
    }

    let scratchCreateTextureJob = new CreateTextureJob();

    function mapJointNames(forest, nodes) {
        let length = forest.length;
        let jointNodes = {};
        for (let i = 0; i < length; ++i) {
            let stack = [forest[i]]; // Push root node of tree

            while (stack.length > 0) {
                let id = stack.pop();
                let n = nodes[id];

                if (Cesium.defined(n)) {
                    jointNodes[id] = id;
                }

                let children = n.children;
                if (Cesium.defined(children)) {
                    let childrenLength = children.length;
                    for (let k = 0; k < childrenLength; ++k) {
                        stack.push(children[k]);
                    }
                }
            }
        }
        return jointNodes;
    }

    function createJoints(model, runtimeSkins) {
        let gltf = model.gltf;
        let skins = gltf.skins;
        let nodes = gltf.nodes;
        let runtimeNodes = model._runtime.nodes;

        let skinnedNodesIds = model._loadResources.skinnedNodesIds;
        let length = skinnedNodesIds.length;
        for (let j = 0; j < length; ++j) {
            let id = skinnedNodesIds[j];
            let skinnedNode = runtimeNodes[id];
            let node = nodes[id];

            let runtimeSkin = runtimeSkins[node.skin];
            skinnedNode.inverseBindMatrices = runtimeSkin.inverseBindMatrices;
            skinnedNode.bindShapeMatrix = runtimeSkin.bindShapeMatrix;

            // 1. Find nodes with the names in node.skeletons (the node's skeletons)
            // 2. These nodes form the root nodes of the forest to search for each joint in skin.jointNames.  This search uses jointName, not the node's name.
            // 3. Search for the joint name among the gltf node hierarchy instead of the runtime node hierarchy. Child links aren't set up yet for runtime nodes.
            let forest = [];
            let skin = skins[node.skin];
            if (Cesium.defined(skin.skeleton)) {
                forest.push(skin.skeleton);
            }

            let mappedJointNames = mapJointNames(forest, nodes);
            let gltfJointNames = skins[node.skin].joints;
            let jointNamesLength = gltfJointNames.length;
            for (let i = 0; i < jointNamesLength; ++i) {
                let jointName = gltfJointNames[i];
                let nodeId = mappedJointNames[jointName];
                let jointNode = runtimeNodes[nodeId];
                skinnedNode.joints.push(jointNode);
            }
        }
    }

    function createSkins(model) {
        let loadResources = model._loadResources;

        if (loadResources.pendingBufferLoads !== 0) {
            return;
        }

        if (!loadResources.createSkins) {
            return;
        }
        loadResources.createSkins = false;

        let gltf = model.gltf;
        let accessors = gltf.accessors;
        let runtimeSkins = {};

        Cesium.ForEach.skin(gltf, function(skin, id) {
            let accessor = accessors[skin.inverseBindMatrices];

            let bindShapeMatrix;
            if (!Matrix4.equals(skin.bindShapeMatrix, Cesium.Matrix4.IDENTITY)) {
                bindShapeMatrix = Cesium.Matrix4.clone(skin.bindShapeMatrix);
            }

            runtimeSkins[id] = {
                inverseBindMatrices : Cesium.ModelAnimationCache.getSkinInverseBindMatrices(model, accessor),
                bindShapeMatrix : bindShapeMatrix // not used when undefined
            };
        });

        createJoints(model, runtimeSkins);
    }

    function getChannelEvaluator(model, runtimeNode, targetPath, spline) {
        return function(localAnimationTime) {
            //  Workaround for https://github.com/KhronosGroup/glTF/issues/219

            //if (targetPath === 'translation') {
            //    return;
            //}
            if (Cesium.defined(spline)) {
                localAnimationTime = model.clampAnimations ? spline.clampTime(localAnimationTime) : spline.wrapTime(localAnimationTime);
                runtimeNode[targetPath] = spline.evaluate(localAnimationTime, runtimeNode[targetPath]);
                runtimeNode.dirtyNumber = model._maxDirtyNumber;
            }
        };
    }

    function createRuntimeAnimations(model) {
        let loadResources = model._loadResources;

        if (!loadResources.finishedPendingBufferLoads()) {
            return;
        }

        if (!loadResources.createRuntimeAnimations) {
            return;
        }
        loadResources.createRuntimeAnimations = false;

        model._runtime.animations = [];

        let runtimeNodes = model._runtime.nodes;
        let accessors = model.gltf.accessors;

        Cesium.ForEach.animation(model.gltf, function (animation, i) {
            let channels = animation.channels;
            let samplers = animation.samplers;

            // Find start and stop time for the entire animation
            let startTime = Cesium.Number.MAX_VALUE;
            let stopTime = -Cesium.Number.MAX_VALUE;

            let channelsLength = channels.length;
            let channelEvaluators = new Array(channelsLength);

            for (let j = 0; j < channelsLength; ++j) {
                let channel = channels[j];
                let target = channel.target;
                let path = target.path;
                let sampler = samplers[channel.sampler];
                let input = Cesium.ModelAnimationCache.getAnimationParameterValues(model, accessors[sampler.input]);
                let output = Cesium.ModelAnimationCache.getAnimationParameterValues(model, accessors[sampler.output]);

                startTime = Math.min(startTime, input[0]);
                stopTime = Math.max(stopTime, input[input.length - 1]);

                let spline = Cesium.ModelAnimationCache.getAnimationSpline(model, i, animation, channel.sampler, sampler, input, path, output);

                // GLTF_SPEC: Support more targets like materials. https://github.com/KhronosGroup/glTF/issues/142
                channelEvaluators[j] = getChannelEvaluator(model, runtimeNodes[target.node], target.path, spline);
            }

            model._runtime.animations[i] = {
                name : animation.name,
                startTime : startTime,
                stopTime : stopTime,
                channelEvaluators : channelEvaluators
            };
        });
    }

    function createVertexArrays(model, context) {
        let loadResources = model._loadResources;
        if (!loadResources.finishedBuffersCreation() || !loadResources.finishedProgramCreation()
                || !loadResources.createVertexArrays) {
            return;
        }
        loadResources.createVertexArrays = false;

        let rendererBuffers = model._rendererResources.buffers;
        let rendererVertexArrays = model._rendererResources.vertexArrays;
        let gltf = model.gltf;
        let accessors = gltf.accessors;
        Cesium.ForEach.mesh(gltf, function(mesh, meshId) {
            Cesium.ForEach.meshPrimitive(mesh, function(primitive, primitiveId) {
                let attributes = [];
                let attributeLocation;
                let attributeLocations = getAttributeLocations(model, primitive);
                let decodedData = model._decodedData[meshId + '.primitive.' + primitiveId];
                Cesium.ForEach.meshPrimitiveAttribute(primitive, function(accessorId, attributeName) {
                    // Skip if the attribute is not used by the material, e.g., because the asset
                    // was exported with an attribute that wasn't used and the asset wasn't optimized.
                    attributeLocation = attributeLocations[attributeName];
                    if (Cesium.defined(attributeLocation)) {
                        // Use attributes of previously decoded draco geometry
                        if (Cesium.defined(decodedData)) {
                            let decodedAttributes = decodedData.attributes;
                            if (decodedAttributes.hasOwnProperty(attributeName)) {
                                let decodedAttribute = decodedAttributes[attributeName];
                                attributes.push({
                                    index: attributeLocation,
                                    vertexBuffer: rendererBuffers[decodedAttribute.bufferView],
                                    componentsPerAttribute: decodedAttribute.componentsPerAttribute,
                                    componentDatatype: decodedAttribute.componentDatatype,
                                    normalize: decodedAttribute.normalized,
                                    offsetInBytes: decodedAttribute.byteOffset,
                                    strideInBytes: decodedAttribute.byteStride
                                });

                                return;
                            }
                        }

                        let a = accessors[accessorId];
                        let normalize = Cesium.defined(a.normalized) && a.normalized;
                        attributes.push({
                            index: attributeLocation,
                            vertexBuffer: rendererBuffers[a.bufferView],
                            componentsPerAttribute: Cesium.numberOfComponentsForType(a.type),
                            componentDatatype: a.componentType,
                            normalize: normalize,
                            offsetInBytes: a.byteOffset,
                            strideInBytes: Cesium.getAccessorByteStride(gltf, a)
                        });
                    }
                });

                // Add pre-created attributes
                let attribute;
                let attributeName;
                let precreatedAttributes = model._precreatedAttributes;
                if (Cesium.defined(precreatedAttributes)) {
                    for (attributeName in precreatedAttributes) {
                        if (precreatedAttributes.hasOwnProperty(attributeName)) {
                            attributeLocation = attributeLocations[attributeName];
                            if (Cesium.defined(attributeLocation)) {
                                attribute = precreatedAttributes[attributeName];
                                attribute.index = attributeLocation;
                                attributes.push(attribute);
                            }
                        }
                    }
                }

                let indexBuffer;
                if (Cesium.defined(primitive.indices)) {
                    let accessor = accessors[primitive.indices];
                    let bufferView = accessor.bufferView;

                    // Use buffer of previously decoded draco geometry
                    if (Cesium.defined(decodedData)) {
                        bufferView = decodedData.bufferView;
                    }

                    indexBuffer = rendererBuffers[bufferView];
                }
                rendererVertexArrays[meshId + '.primitive.' + primitiveId] = new Cesium.VertexArray({
                    context: context,
                    attributes: attributes,
                    indexBuffer: indexBuffer
                });
            });
        });
    }

    function createRenderStates(model) {
        let loadResources = model._loadResources;
        if (loadResources.createRenderStates) {
            loadResources.createRenderStates = false;

            Cesium.ForEach.material(model.gltf, function (material, materialId) {
                createRenderStateForMaterial(model, material, materialId);
            });
        }
    }

    function createRenderStateForMaterial(model, material, materialId) {
        let rendererRenderStates = model._rendererResources.renderStates;

        let blendEquationSeparate = [
            Cesium.WebGLConstants.FUNC_ADD,
            Cesium.WebGLConstants.FUNC_ADD
        ];
        let blendFuncSeparate = [
            Cesium.WebGLConstants.ONE,
            Cesium.WebGLConstants.ONE_MINUS_SRC_ALPHA,
            Cesium.WebGLConstants.ONE,
            Cesium.WebGLConstants.ONE_MINUS_SRC_ALPHA
        ];

        if (Cesium.defined(material.extensions) && Cesium.defined(material.extensions.KHR_blend)) {
            blendEquationSeparate = material.extensions.KHR_blend.blendEquation;
            blendFuncSeparate = material.extensions.KHR_blend.blendFactors;
        }

        let enableCulling = !material.doubleSided;
        let blendingEnabled = (material.alphaMode === 'BLEND');
        rendererRenderStates[materialId] = Cesium.RenderState.fromCache({
            cull : {
                enabled : enableCulling
            },
            depthTest : {
                enabled : true
            },
            depthMask : !blendingEnabled,
            blending : {
                enabled : blendingEnabled,
                equationRgb : blendEquationSeparate[0],
                equationAlpha : blendEquationSeparate[1],
                functionSourceRgb : blendFuncSeparate[0],
                functionDestinationRgb : blendFuncSeparate[1],
                functionSourceAlpha : blendFuncSeparate[2],
                functionDestinationAlpha : blendFuncSeparate[3]
            }
        });
    }

    let gltfUniformsFromNode = {
        MODEL : function(uniformState, model, runtimeNode) {
            return function() {
                return runtimeNode.computedMatrix;
            };
        },
        VIEW : function(uniformState, model, runtimeNode) {
            return function() {
                return uniformState.view;
            };
        },
        PROJECTION : function(uniformState, model, runtimeNode) {
            return function() {
                return uniformState.projection;
            };
        },
        MODELVIEW : function(uniformState, model, runtimeNode) {
            let mv = new Cesium.Matrix4();
            return function() {
                return Cesium.Matrix4.multiplyTransformation(uniformState.view, runtimeNode.computedMatrix, mv);
            };
        },
        CESIUM_RTC_MODELVIEW : function(uniformState, model, runtimeNode) {
            // CESIUM_RTC extension
            let mvRtc = new Cesium.Matrix4();
            return function() {
                Cesium.Matrix4.multiplyTransformation(uniformState.view, runtimeNode.computedMatrix, mvRtc);
                return Cesium.Matrix4.setTranslation(mvRtc, model._rtcCenterEye, mvRtc);
            };
        },
        MODELVIEWPROJECTION : function(uniformState, model, runtimeNode) {
            let mvp = new Cesium.Matrix4();
            return function() {
                Cesium.Matrix4.multiplyTransformation(uniformState.view, runtimeNode.computedMatrix, mvp);
                return Cesium.Matrix4.multiply(uniformState._projection, mvp, mvp);
            };
        },
        MODELINVERSE : function(uniformState, model, runtimeNode) {
            let mInverse = new Cesium.Matrix4();
            return function() {
                return Cesium.Matrix4.inverse(runtimeNode.computedMatrix, mInverse);
            };
        },
        VIEWINVERSE : function(uniformState, model) {
            return function() {
                return uniformState.inverseView;
            };
        },
        PROJECTIONINVERSE : function(uniformState, model, runtimeNode) {
            return function() {
                return uniformState.inverseProjection;
            };
        },
        MODELVIEWINVERSE : function(uniformState, model, runtimeNode) {
            let mv = new Cesium.Matrix4();
            let mvInverse = new Cesium.Matrix4();
            return function() {
                Cesium.Matrix4.multiplyTransformation(uniformState.view, runtimeNode.computedMatrix, mv);
                return Cesium.Matrix4.inverse(mv, mvInverse);
            };
        },
        MODELVIEWPROJECTIONINVERSE : function(uniformState, model, runtimeNode) {
            let mvp = new Cesium.Matrix4();
            let mvpInverse = new Cesium.Matrix4();
            return function() {
                Cesium.Matrix4.multiplyTransformation(uniformState.view, runtimeNode.computedMatrix, mvp);
                Cesium.Matrix4.multiply(uniformState._projection, mvp, mvp);
                return Cesium.Matrix4.inverse(mvp, mvpInverse);
            };
        },
        MODELINVERSETRANSPOSE : function(uniformState, model, runtimeNode) {
            let mInverse = new Cesium.Matrix4();
            let mInverseTranspose = new Cesium.Matrix3();
            return function() {
                Cesium.Matrix4.inverse(runtimeNode.computedMatrix, mInverse);
                Cesium.Matrix4.getRotation(mInverse, mInverseTranspose);
                return Cesium.Matrix3.transpose(mInverseTranspose, mInverseTranspose);
            };
        },
        MODELVIEWINVERSETRANSPOSE : function(uniformState, model, runtimeNode) {
            let mv = new Cesium.Matrix4();
            let mvInverse = new Cesium.Matrix4();
            let mvInverseTranspose = new Cesium.Matrix3();
            return function() {
                Cesium.Matrix4.multiplyTransformation(uniformState.view, runtimeNode.computedMatrix, mv);
                Cesium.Matrix4.inverse(mv, mvInverse);
                Cesium.Matrix4.getRotation(mvInverse, mvInverseTranspose);
                return Cesium.Matrix3.transpose(mvInverseTranspose, mvInverseTranspose);
            };
        },
        VIEWPORT : function(uniformState, model, runtimeNode) {
            return function() {
                return uniformState.viewportCartesian4;
            };
        }
    };

    function getUniformFunctionFromSource(source, model, semantic, uniformState) {
        let runtimeNode = model._runtime.nodes[source];
        return gltfUniformsFromNode[semantic](uniformState, model, runtimeNode);
    }

    function createUniformsForMaterial(model, material, technique, instanceValues, context, textures, defaultTexture) {
        let uniformMap = {};
        let uniformValues = {};
        let jointMatrixUniformName;
        let morphWeightsUniformName;

        Cesium.ForEach.techniqueUniform(technique, function(uniform, uniformName) {
            // GLTF_SPEC: This does not take into account uniform arrays,
            // indicated by uniforms with a count property.
            //
            // https://github.com/KhronosGroup/glTF/issues/258

            // GLTF_SPEC: In this implementation, material parameters with a
            // semantic or targeted via a source (for animation) are not
            // targetable for material animations.  Is this too strict?
            //
            // https://github.com/KhronosGroup/glTF/issues/142

            let uv;
            if (Cesium.defined(instanceValues) && Cesium.defined(instanceValues[uniformName])) {
                // Parameter overrides by the instance technique
                uv = Cesium.ModelUtility.createUniformFunction(uniform.type, instanceValues[uniformName], textures, defaultTexture);
                uniformMap[uniformName] = uv.func;
                uniformValues[uniformName] = uv;
            } else if (Cesium.defined(uniform.node)) {
                uniformMap[uniformName] = getUniformFunctionFromSource(uniform.node, model, uniform.semantic, context.uniformState);
            } else if (Cesium.defined(uniform.semantic)) {
                if (uniform.semantic === 'JOINTMATRIX') {
                    jointMatrixUniformName = uniformName;
                } else if (uniform.semantic === 'MORPHWEIGHTS') {
                    morphWeightsUniformName = uniformName;
                } else if (uniform.semantic === 'ALPHACUTOFF') {
                    // The material's alphaCutoff value uses a uniform with semantic ALPHACUTOFF.
                    // A uniform with this semantic will ignore the instance or default values.
                    let alphaMode = material.alphaMode;
                    if (Cesium.defined(alphaMode) && alphaMode === 'MASK') {
                        let alphaCutoffValue = Cesium.defaultValue(material.alphaCutoff, 0.5);
                        uv = Cesium.ModelUtility.createUniformFunction(uniform.type, alphaCutoffValue, textures, defaultTexture);
                        uniformMap[uniformName] = uv.func;
                        uniformValues[uniformName] = uv;
                    }
                } else {
                    // Map glTF semantic to Cesium automatic uniform
                    uniformMap[uniformName] = Cesium.ModelUtility.getGltfSemanticUniforms()[uniform.semantic](context.uniformState, model);
                }
            } else if (Cesium.defined(uniform.value)) {
                // Technique value that isn't overridden by a material
                let uv2 = Cesium.ModelUtility.createUniformFunction(uniform.type, uniform.value, textures, defaultTexture);
                uniformMap[uniformName] = uv2.func;
                uniformValues[uniformName] = uv2;
            }
        });

        return {
            map : uniformMap,
            values : uniformValues,
            jointMatrixUniformName : jointMatrixUniformName,
            morphWeightsUniformName : morphWeightsUniformName
        };
    }

    function createUniformMaps(model, context) {
        let loadResources = model._loadResources;

        if (!loadResources.finishedProgramCreation()) {
            return;
        }

        if (!loadResources.createUniformMaps) {
            return;
        }
        loadResources.createUniformMaps = false;

        let gltf = model.gltf;
        let techniques = model._sourceTechniques;
        let uniformMaps = model._uniformMaps;

        let textures = model._rendererResources.textures;
        let defaultTexture = model._defaultTexture;

        Cesium.ForEach.material(gltf, function (material, materialId) {
            let modelMaterial = model._runtime.materialsById[materialId];
            let technique = techniques[modelMaterial._technique];
            let instanceValues = modelMaterial._values;

            let uniforms = createUniformsForMaterial(model, material, technique, instanceValues, context, textures, defaultTexture);

            let u = uniformMaps[materialId];
            u.uniformMap = uniforms.map;                          // uniform name -> function for the renderer
            u.values = uniforms.values;                           // material parameter name -> ModelMaterial for modifying the parameter at runtime
            u.jointMatrixUniformName = uniforms.jointMatrixUniformName;
            u.morphWeightsUniformName = uniforms.morphWeightsUniformName;
        });
    }

    function createUniformsForDracoQuantizedAttributes(decodedData) {
        return Cesium.ModelUtility.createUniformsForDracoQuantizedAttributes(decodedData.attributes);
    }

    function createUniformsForQuantizedAttributes(model, primitive) {
        let programId = getProgramForPrimitive(model, primitive);
        let quantizedUniforms = model._quantizedUniforms[programId];
        return Cesium.ModelUtility.createUniformsForQuantizedAttributes(model.gltf, primitive, quantizedUniforms);
    }

    function createPickColorFunction(color) {
        return function() {
            return color;
        };
    }

    function createJointMatricesFunction(runtimeNode) {
        return function() {
            return runtimeNode.computedJointMatrices;
        };
    }

    function createMorphWeightsFunction(runtimeNode) {
        return function() {
            return runtimeNode.weights;
        };
    }

    function createSilhouetteColorFunction(model) {
        return function() {
            return model.silhouetteColor;
        };
    }

    function createSilhouetteSizeFunction(model) {
        return function() {
            return model.silhouetteSize;
        };
    }

    function createColorFunction(model) {
        return function() {
            return model.color;
        };
    }

    let scratchClippingPlaneMatrix = new Cesium.Matrix4();
    function createClippingPlanesMatrixFunction(model) {
        return function() {
            let clippingPlanes = model.clippingPlanes;
            if (!Cesium.defined(clippingPlanes) && !Cesium.defined(model._sphericalHarmonicCoefficients) && !Cesium.defined(model._specularEnvironmentMaps)) {
                return Cesium.Matrix4.IDENTITY;
            }
            let modelMatrix = Cesium.defined(clippingPlanes) ? clippingPlanes.modelMatrix : Matrix4.IDENTITY;
            return Matrix4.multiply(model._clippingPlaneModelViewMatrix, modelMatrix, scratchClippingPlaneMatrix);
        };
    }

    function createClippingPlanesFunction(model) {
        return function() {
            let clippingPlanes = model.clippingPlanes;
            return (!Cesium.defined(clippingPlanes) || !clippingPlanes.enabled) ? model._defaultTexture : clippingPlanes.texture;
        };
    }

    function createClippingPlanesEdgeStyleFunction(model) {
        return function() {
            let clippingPlanes = model.clippingPlanes;
            if (!Cesium.defined(clippingPlanes)) {
                return Cesium.Color.WHITE.withAlpha(0.0);
            }

            let style = Cesium.Color.clone(clippingPlanes.edgeColor);
            style.alpha = clippingPlanes.edgeWidth;
            return style;
        };
    }

    function createColorBlendFunction(model) {
        return function() {
            return Cesium.ColorBlendMode.getColorBlend(model.colorBlendMode, model.colorBlendAmount);
        };
    }

    function createIBLFactorFunction(model) {
        return function() {
            return model._imageBasedLightingFactor;
        };
    }

    function createLightColorFunction(model) {
        return function() {
            return model._lightColor;
        };
    }

    function createLuminanceAtZenithFunction(model) {
        return function() {
            return model.luminanceAtZenith;
        };
    }

    function createSphericalHarmonicCoefficientsFunction(model) {
        return function() {
            return model._sphericalHarmonicCoefficients;
        };
    }

    function createSpecularEnvironmentMapFunction(model) {
        return function() {
            return model._specularEnvironmentMapAtlas.texture;
        };
    }

    function createSpecularEnvironmentMapSizeFunction(model) {
        return function() {
            return model._specularEnvironmentMapAtlas.texture.dimensions;
        };
    }

    function createSpecularEnvironmentMapLOD(model) {
        return function() {
            return model._specularEnvironmentMapAtlas.maximumMipmapLevel;
        };
    }

    function triangleCountFromPrimitiveIndices(primitive, indicesCount) {
        switch (primitive.mode) {
            case Cesium.PrimitiveType.TRIANGLES:
                return (indicesCount / 3);
            case Cesium.PrimitiveType.TRIANGLE_STRIP:
            case Cesium.PrimitiveType.TRIANGLE_FAN:
                return Math.max(indicesCount - 2, 0);
            default:
                return 0;
        }
    }

    function createCommand(model, gltfNode, runtimeNode, context, scene3DOnly) {
        let nodeCommands = model._nodeCommands;
        let pickIds = model._pickIds;
        let allowPicking = model.allowPicking;
        let runtimeMeshesByName = model._runtime.meshesByName;

        let resources = model._rendererResources;
        let rendererVertexArrays = resources.vertexArrays;
        let rendererPrograms = resources.programs;
        let rendererRenderStates = resources.renderStates;
        let uniformMaps = model._uniformMaps;

        let gltf = model.gltf;
        let accessors = gltf.accessors;
        let gltfMeshes = gltf.meshes;

        let id = gltfNode.mesh;
        let mesh = gltfMeshes[id];

        let primitives = mesh.primitives;
        let length = primitives.length;

        // The glTF node hierarchy is a DAG so a node can have more than one
        // parent, so a node may already have commands.  If so, append more
        // since they will have a different model matrix.

        for (let i = 0; i < length; ++i) {
            let primitive = primitives[i];
            let ix = accessors[primitive.indices];
            let material = model._runtime.materialsById[primitive.material];
            let programId = material._program;
            let decodedData = model._decodedData[id + '.primitive.' + i];

            let boundingSphere;
            let positionAccessor = primitive.attributes.POSITION;
            if (Cesium.defined(positionAccessor)) {
                let minMax = Cesium.ModelUtility.getAccessorMinMax(gltf, positionAccessor);
                boundingSphere = Cesium.BoundingSphere.fromCornerPoints(Cesium.Cartesian3.fromArray(minMax.min), Cesium.Cartesian3.fromArray(minMax.max));
            }

            let vertexArray = rendererVertexArrays[id + '.primitive.' + i];
            let offset;
            let count;

            // Use indices of the previously decoded Draco geometry.
            if (Cesium.defined(decodedData)) {
                count = decodedData.numberOfIndices;
                offset = 0;
            } else if (Cesium.defined(ix)) {
                count = ix.count;
                offset = (ix.byteOffset / Cesium.IndexDatatype.getSizeInBytes(ix.componentType)); // glTF has offset in bytes.  Cesium has offsets in indices
            } else {
                let positions = accessors[primitive.attributes.POSITION];
                count = positions.count;
                offset = 0;
            }

            // Update model triangle count using number of indices
            model._trianglesLength += triangleCountFromPrimitiveIndices(primitive, count);

            let um = uniformMaps[primitive.material];
            let uniformMap = um.uniformMap;
            if (Cesium.defined(um.jointMatrixUniformName)) {
                let jointUniformMap = {};
                jointUniformMap[um.jointMatrixUniformName] = createJointMatricesFunction(runtimeNode);

                uniformMap = Cesium.combine(uniformMap, jointUniformMap);
            }
            if (Cesium.defined(um.morphWeightsUniformName)) {
                let morphWeightsUniformMap = {};
                morphWeightsUniformMap[um.morphWeightsUniformName] = createMorphWeightsFunction(runtimeNode);

                uniformMap = Cesium.combine(uniformMap, morphWeightsUniformMap);
            }

            uniformMap = Cesium.combine(uniformMap, {
                gltf_color : createColorFunction(model),
                gltf_colorBlend : createColorBlendFunction(model),
                gltf_clippingPlanes : createClippingPlanesFunction(model),
                gltf_clippingPlanesEdgeStyle : createClippingPlanesEdgeStyleFunction(model),
                gltf_clippingPlanesMatrix : createClippingPlanesMatrixFunction(model),
                gltf_iblFactor : createIBLFactorFunction(model),
                gltf_lightColor : createLightColorFunction(model),
                gltf_sphericalHarmonicCoefficients : createSphericalHarmonicCoefficientsFunction(model),
                gltf_specularMap : createSpecularEnvironmentMapFunction(model),
                gltf_specularMapSize : createSpecularEnvironmentMapSizeFunction(model),
                gltf_maxSpecularLOD : createSpecularEnvironmentMapLOD(model),
                gltf_luminanceAtZenith : createLuminanceAtZenithFunction(model)
            });

            // Allow callback to modify the uniformMap
            if (Cesium.defined(model._uniformMapLoaded)) {
                uniformMap = model._uniformMapLoaded(uniformMap, programId, runtimeNode);
            }

            // Add uniforms for decoding quantized attributes if used
            let quantizedUniformMap = {};
            if (model.extensionsUsed.WEB3D_quantized_attributes) {
                quantizedUniformMap = createUniformsForQuantizedAttributes(model, primitive);
            } else if (model._dequantizeInShader && Cesium.defined(decodedData)) {
                quantizedUniformMap = createUniformsForDracoQuantizedAttributes(decodedData);
            }
            uniformMap = Cesium.combine(uniformMap, quantizedUniformMap);

            let rs = rendererRenderStates[primitive.material];
            let isTranslucent = rs.blending.enabled;

            let owner = model._pickObject;
            if (!Cesium.defined(owner)) {
                owner = {
                    primitive : model,
                    id : model.id,
                    node : runtimeNode.publicNode,
                    mesh : runtimeMeshesByName[mesh.name]
                };
            }

            let castShadows = Cesium.ShadowMode.castShadows(model._shadows);
            let receiveShadows = Cesium.ShadowMode.receiveShadows(model._shadows);

            let pickId;
            let pickOid;
            if (allowPicking && !Cesium.defined(model._uniformMapLoaded)) {
                pickId = context.createPickId(owner);
                pickIds.push(pickId);
                let pickUniforms = {
                    czm_pickColor : createPickColorFunction(pickId.color)
                };
                uniformMap = Cesium.combine(uniformMap, pickUniforms);
            }

            if (allowPicking) {
                if (Cesium.defined(model._pickIdLoaded) && Cesium.defined(model._uniformMapLoaded)) {
                    pickId = model._pickIdLoaded();
                } else {
                    pickId = 'czm_pickColor';
                }
                //fgy
                if(Cesium.defined(model._pickOidLoaded )&& Cesium.defined(model._uniformMapLoaded)){
                    pickOid = model._pickOidLoaded();
                }else{
                    pickOid = undefined;
                }
            }
            //韩彦生 添加分析标识
            let command = new Cesium.DrawCommand({
                boundingVolume : new Cesium.BoundingSphere(), // updated in update()
                cull : model.cull,
                modelMatrix : new Cesium.Matrix4(),           // computed in update()
                primitiveType : primitive.mode,
                vertexArray : vertexArray,
                count : count,
                offset : offset,
                shaderProgram : rendererPrograms[programId],
                castShadows : castShadows,
                receiveShadows : receiveShadows,
                uniformMap : uniformMap,
                renderState : rs,
                owner : owner,
                pass : isTranslucent ? Cesium.Pass.TRANSLUCENT : model.opaquePass,
                pickId : pickId,
                pickOid: pickOid
            });

            let command2D;
            if (!scene3DOnly) {
                command2D = Cesium.DrawCommand.shallowClone(command);
                command2D.boundingVolume = new Cesium.BoundingSphere(); // updated in update()
                command2D.modelMatrix = new Cesium.Matrix4();           // updated in update()
            }

            let nodeCommand = {
                show : true,
                boundingSphere : boundingSphere,
                command : command,
                command2D : command2D,
                // Generated on demand when silhouette size is greater than 0.0 and silhouette alpha is greater than 0.0
                silhouetteModelCommand : undefined,
                silhouetteModelCommand2D : undefined,
                silhouetteColorCommand : undefined,
                silhouetteColorCommand2D : undefined,
                // Generated on demand when color alpha is less than 1.0
                translucentCommand : undefined,
                translucentCommand2D : undefined,
                // For updating node commands on shader reconstruction
                programId : programId
            };
            runtimeNode.commands.push(nodeCommand);
            nodeCommands.push(nodeCommand);
        }
    }

    function createRuntimeNodes(model, context, scene3DOnly) {
        let loadResources = model._loadResources;

        if (!loadResources.finishedEverythingButTextureCreation()) {
            return;
        }

        if (!loadResources.createRuntimeNodes) {
            return;
        }
        loadResources.createRuntimeNodes = false;

        let rootNodes = [];
        let runtimeNodes = model._runtime.nodes;

        let gltf = model.gltf;
        let nodes = gltf.nodes;
        let skins = gltf.skins;

        let scene = gltf.scenes[gltf.scene];
        let sceneNodes = scene.nodes;
        let length = sceneNodes.length;

        let stack = [];
        let seen = {};

        for (let i = 0; i < length; ++i) {
            stack.push({
                parentRuntimeNode : undefined,
                gltfNode : nodes[sceneNodes[i]],
                id : sceneNodes[i]
            });

            let skeletonIds = [];
            while (stack.length > 0) {
                let n = stack.pop();
                seen[n.id] = true;
                let parentRuntimeNode = n.parentRuntimeNode;
                let gltfNode = n.gltfNode;

                // Node hierarchy is a DAG so a node can have more than one parent so it may already exist
                let runtimeNode = runtimeNodes[n.id];
                if (runtimeNode.parents.length === 0) {
                    if (Cesium.defined(gltfNode.matrix)) {
                        runtimeNode.matrix = Cesium.Matrix4.fromColumnMajorArray(gltfNode.matrix);
                    } else {
                        // TRS converted to Cesium types
                        let rotation = gltfNode.rotation;
                        runtimeNode.translation = Cesium.Cartesian3.fromArray(gltfNode.translation);
                        runtimeNode.rotation = Cesium.Quaternion.unpack(rotation);
                        runtimeNode.scale = Cesium.Cartesian3.fromArray(gltfNode.scale);
                    }
                }

                if (Cesium.defined(parentRuntimeNode)) {
                    parentRuntimeNode.children.push(runtimeNode);
                    runtimeNode.parents.push(parentRuntimeNode);
                } else {
                    rootNodes.push(runtimeNode);
                }

                if (Cesium.defined(gltfNode.mesh)) {
                    createCommand(model, gltfNode, runtimeNode, context, scene3DOnly);
                }

                let children = gltfNode.children;
                if (Cesium.defined(children)) {
                    let childrenLength = children.length;
                    for (let j = 0; j < childrenLength; j++) {
                        let childId = children[j];
                        if (!seen[childId]) {
                            stack.push({
                                parentRuntimeNode : runtimeNode,
                                gltfNode : nodes[childId],
                                id : children[j]
                            });
                        }
                    }
                }

                let skin = gltfNode.skin;
                if (Cesium.defined(skin)) {
                    skeletonIds.push(skins[skin].skeleton);
                }

                if (stack.length === 0) {
                    for (let k = 0; k < skeletonIds.length; k++) {
                        let skeleton = skeletonIds[k];
                        if (!seen[skeleton]) {
                            stack.push({
                                parentRuntimeNode : undefined,
                                gltfNode : nodes[skeleton],
                                id : skeleton
                            });
                        }
                    }
                }
            }
        }

        model._runtime.rootNodes = rootNodes;
        model._runtime.nodes = runtimeNodes;
    }

    function getGeometryByteLength(buffers) {
        let memory = 0;
        for (let id in buffers) {
            if (buffers.hasOwnProperty(id)) {
                memory += buffers[id].sizeInBytes;
            }
        }
        return memory;
    }

    function getTexturesByteLength(textures) {
        let memory = 0;
        for (let id in textures) {
            if (textures.hasOwnProperty(id)) {
                memory += textures[id].sizeInBytes;
            }
        }
        return memory;
    }

    function createResources(model, frameState) {
        let context = frameState.context;
        let scene3DOnly = frameState.scene3DOnly;
        let quantizedVertexShaders = model._quantizedVertexShaders;
        let toClipCoordinates = model._toClipCoordinatesGLSL = {};
        let techniques = model._sourceTechniques;
        let programs = model._sourcePrograms;

        let resources = model._rendererResources;
        let shaders = resources.sourceShaders;
        if (model._loadRendererResourcesFromCache) {
            shaders = resources.sourceShaders = model._cachedRendererResources.sourceShaders;
        }

        for (let techniqueId in techniques) {
            if (techniques.hasOwnProperty(techniqueId)) {
                let programId = techniques[techniqueId].program;
                let program = programs[programId];
                let shader = shaders[program.vertexShader];

                Cesium.ModelUtility.checkSupportedGlExtensions(program.glExtensions, context);

                if (model.extensionsUsed.WEB3D_quantized_attributes || model._dequantizeInShader) {
                    let quantizedVS = quantizedVertexShaders[programId];
                    if (!Cesium.defined(quantizedVS)) {
                        quantizedVS = modifyShaderForQuantizedAttributes(shader, programId, model);
                        quantizedVertexShaders[programId] = quantizedVS;
                    }
                    shader = quantizedVS;
                }

                shader = modifyShader(shader, programId, model._vertexShaderLoaded);
                toClipCoordinates[programId] = Cesium.ModelUtility.toClipCoordinatesGLSL(model.gltf, shader);
            }
        }

        if (model._loadRendererResourcesFromCache) {
            let cachedResources = model._cachedRendererResources;

            resources.buffers = cachedResources.buffers;
            resources.vertexArrays = cachedResources.vertexArrays;
            resources.programs = cachedResources.programs;
            resources.silhouettePrograms = cachedResources.silhouettePrograms;
            resources.textures = cachedResources.textures;
            resources.samplers = cachedResources.samplers;
            resources.renderStates = cachedResources.renderStates;

            // Vertex arrays are unique to this model, create instead of using the cache.
            if (Cesium.defined(model._precreatedAttributes)) {
                createVertexArrays(model, context);
            }

            model._cachedGeometryByteLength += getGeometryByteLength(cachedResources.buffers);
            model._cachedTexturesByteLength += getTexturesByteLength(cachedResources.textures);
        } else {
            createBuffers(model, frameState); // using glTF bufferViews
            createPrograms(model, frameState);
            createSamplers(model, context);
            loadTexturesFromBufferViews(model);
            createTextures(model, frameState);
        }

        createSkins(model);
        createRuntimeAnimations(model);

        if (!model._loadRendererResourcesFromCache) {
            createVertexArrays(model, context); // using glTF meshes
            createRenderStates(model); // using glTF materials/techniques/states
            // Long-term, we might not cache render states if they could change
            // due to an animation, e.g., a uniform going from opaque to transparent.
            // Could use copy-on-write if it is worth it.  Probably overkill.
        }

        createUniformMaps(model, context);               // using glTF materials/techniques
        createRuntimeNodes(model, context, scene3DOnly); // using glTF scene
    }

    ///////////////////////////////////////////////////////////////////////////

    function getNodeMatrix(node, result) {
        let publicNode = node.publicNode;
        let publicMatrix = publicNode.matrix;

        if (publicNode.useMatrix && Cesium.defined(publicMatrix)) {
            // Public matrix overrides original glTF matrix and glTF animations
            Cesium.Matrix4.clone(publicMatrix, result);
        } else if (Cesium.defined(node.matrix)) {
            Cesium.Matrix4.clone(node.matrix, result);
        } else {
            Cesium.Matrix4.fromTranslationQuaternionRotationScale(node.translation, node.rotation, node.scale, result);
            // Keep matrix returned by the node in-sync if the node is targeted by an animation.  Only TRS nodes can be targeted.
            publicNode.setMatrix(result);
        }
    }

    let scratchNodeStack = [];
    let scratchComputedTranslation = new Cesium.Cartesian4();
    let scratchComputedMatrixIn2D = new Cesium.Matrix4();

    function updateNodeHierarchyModelMatrix(model, modelTransformChanged, justLoaded, projection) {
        let maxDirtyNumber = model._maxDirtyNumber;

        let rootNodes = model._runtime.rootNodes;
        let length = rootNodes.length;

        let nodeStack = scratchNodeStack;
        let computedModelMatrix = model._computedModelMatrix;

        if ((model._mode !== Cesium.SceneMode.SCENE3D) && !model._ignoreCommands) {
            let translation = Cesium.Matrix4.getColumn(computedModelMatrix, 3, scratchComputedTranslation);
            if (!Cesium.Cartesian4.equals(translation, Cesium.Cartesian4.UNIT_W)) {
                computedModelMatrix = Cesium.Transforms.basisTo2D(projection, computedModelMatrix, scratchComputedMatrixIn2D);
                model._rtcCenter = model._rtcCenter3D;
            } else {
                let center = model.boundingSphere.center;
                let to2D = Cesium.Transforms.wgs84To2DModelMatrix(projection, center, scratchComputedMatrixIn2D);
                computedModelMatrix = Cesium.Matrix4.multiply(to2D, computedModelMatrix, scratchComputedMatrixIn2D);

                if (Cesium.defined(model._rtcCenter)) {
                    Cesium.Matrix4.setTranslation(computedModelMatrix, Cesium.Cartesian4.UNIT_W, computedModelMatrix);
                    model._rtcCenter = model._rtcCenter2D;
                }
            }
        }

        for (let i = 0; i < length; ++i) {
            let n = rootNodes[i];

            getNodeMatrix(n, n.transformToRoot);
            nodeStack.push(n);

            while (nodeStack.length > 0) {
                n = nodeStack.pop();
                let transformToRoot = n.transformToRoot;
                let commands = n.commands;

                if ((n.dirtyNumber === maxDirtyNumber) || modelTransformChanged || justLoaded) {
                    let nodeMatrix = Cesium.Matrix4.multiplyTransformation(computedModelMatrix, transformToRoot, n.computedMatrix);
                    let commandsLength = commands.length;
                    if (commandsLength > 0) {
                        // Node has meshes, which has primitives.  Update their commands.
                        for (let j = 0; j < commandsLength; ++j) {
                            let primitiveCommand = commands[j];
                            let command = primitiveCommand.command;
                            Cesium.Matrix4.clone(nodeMatrix, command.modelMatrix);

                            // PERFORMANCE_IDEA: Can use transformWithoutScale if no node up to the root has scale (including animation)
                            Cesium.BoundingSphere.transform(primitiveCommand.boundingSphere, command.modelMatrix, command.boundingVolume);

                            if (Cesium.defined(model._rtcCenter)) {
                                Cesium.Cartesian3.add(model._rtcCenter, command.boundingVolume.center, command.boundingVolume.center);
                            }

                            // If the model crosses the IDL in 2D, it will be drawn in one viewport, but part of it
                            // will be clipped by the viewport. We create a second command that translates the model
                            // model matrix to the opposite side of the map so the part that was clipped in one viewport
                            // is drawn in the other.
                            command = primitiveCommand.command2D;
                            if (Cesium.defined(command) && model._mode === Cesium.SceneMode.SCENE2D) {
                                Cesium.Matrix4.clone(nodeMatrix, command.modelMatrix);
                                command.modelMatrix[13] -= Cesium.Math.sign(command.modelMatrix[13]) * 2.0 * Cesium.Math.PI * projection.ellipsoid.maximumRadius;
                                Cesium.BoundingSphere.transform(primitiveCommand.boundingSphere, command.modelMatrix, command.boundingVolume);
                            }
                        }
                    }
                }

                let children = n.children;
                if (Cesium.defined(children)) {
                    let childrenLength = children.length;
                    for (let k = 0; k < childrenLength; ++k) {
                        let child = children[k];

                        // A node's transform needs to be updated if
                        // - It was targeted for animation this frame, or
                        // - Any of its ancestors were targeted for animation this frame

                        // PERFORMANCE_IDEA: if a child has multiple parents and only one of the parents
                        // is dirty, all the subtrees for each child instance will be dirty; we probably
                        // won't see this in the wild often.
                        child.dirtyNumber = Math.max(child.dirtyNumber, n.dirtyNumber);

                        if ((child.dirtyNumber === maxDirtyNumber) || justLoaded) {
                            // Don't check for modelTransformChanged since if only the model's model matrix changed,
                            // we do not need to rebuild the local transform-to-root, only the final
                            // [model's-model-matrix][transform-to-root] above.
                            getNodeMatrix(child, child.transformToRoot);
                            Cesium.Matrix4.multiplyTransformation(transformToRoot, child.transformToRoot, child.transformToRoot);
                        }

                        nodeStack.push(child);
                    }
                }
            }
        }

        ++model._maxDirtyNumber;
    }

    let scratchObjectSpace = new Cesium.Matrix4();

    function applySkins(model) {
        let skinnedNodes = model._runtime.skinnedNodes;
        let length = skinnedNodes.length;

        for (let i = 0; i < length; ++i) {
            let node = skinnedNodes[i];

            scratchObjectSpace = Cesium.Matrix4.inverseTransformation(node.transformToRoot, scratchObjectSpace);

            let computedJointMatrices = node.computedJointMatrices;
            let joints = node.joints;
            let bindShapeMatrix = node.bindShapeMatrix;
            let inverseBindMatrices = node.inverseBindMatrices;
            let inverseBindMatricesLength = inverseBindMatrices.length;

            for (let m = 0; m < inverseBindMatricesLength; ++m) {
                // [joint-matrix] = [node-to-root^-1][joint-to-root][inverse-bind][bind-shape]
                if (!Cesium.defined(computedJointMatrices[m])) {
                    computedJointMatrices[m] = new Cesium.Matrix4();
                }
                computedJointMatrices[m] = Cesium.Matrix4.multiplyTransformation(scratchObjectSpace, joints[m].transformToRoot, computedJointMatrices[m]);
                computedJointMatrices[m] = Cesium.Matrix4.multiplyTransformation(computedJointMatrices[m], inverseBindMatrices[m], computedJointMatrices[m]);
                if (Cesium.defined(bindShapeMatrix)) {
                    // Optimization for when bind shape matrix is the identity.
                    computedJointMatrices[m] = Cesium.Matrix4.multiplyTransformation(computedJointMatrices[m], bindShapeMatrix, computedJointMatrices[m]);
                }
            }
        }
    }

    function updatePerNodeShow(model) {
        // Totally not worth it, but we could optimize this:
        // http://blogs.agi.com/insight3d/index.php/2008/02/13/deletion-in-bounding-volume-hierarchies/

        let rootNodes = model._runtime.rootNodes;
        let length = rootNodes.length;

        let nodeStack = scratchNodeStack;

        for (let i = 0; i < length; ++i) {
            let n = rootNodes[i];
            n.computedShow = n.publicNode.show;
            nodeStack.push(n);

            while (nodeStack.length > 0) {
                n = nodeStack.pop();
                let show = n.computedShow;

                let nodeCommands = n.commands;
                let nodeCommandsLength = nodeCommands.length;
                for (let j = 0; j < nodeCommandsLength; ++j) {
                    nodeCommands[j].show = show;
                }
                // if commandsLength is zero, the node has a light or camera

                let children = n.children;
                if (Cesium.defined(children)) {
                    let childrenLength = children.length;
                    for (let k = 0; k < childrenLength; ++k) {
                        let child = children[k];
                        // Parent needs to be shown for child to be shown.
                        child.computedShow = show && child.publicNode.show;
                        nodeStack.push(child);
                    }
                }
            }
        }
    }

    function updatePickIds(model, context) {
        let id = model.id;
        if (model._id !== id) {
            model._id = id;

            let pickIds = model._pickIds;
            let length = pickIds.length;
            for (let i = 0; i < length; ++i) {
                pickIds[i].object.id = id;
            }
        }
    }

    function updateWireframe(model) {
        if (model._debugWireframe !== model.debugWireframe) {
            model._debugWireframe = model.debugWireframe;

            // This assumes the original primitive was TRIANGLES and that the triangles
            // are connected for the wireframe to look perfect.
            let primitiveType = model.debugWireframe ? Cesium.PrimitiveType.LINES : Cesium.PrimitiveType.TRIANGLES;
            let nodeCommands = model._nodeCommands;
            let length = nodeCommands.length;

            for (let i = 0; i < length; ++i) {
                nodeCommands[i].command.primitiveType = primitiveType;
            }
        }
    }

    function updateShowBoundingVolume(model) {
        if (model.debugShowBoundingVolume !== model._debugShowBoundingVolume) {
            model._debugShowBoundingVolume = model.debugShowBoundingVolume;

            let debugShowBoundingVolume = model.debugShowBoundingVolume;
            let nodeCommands = model._nodeCommands;
            let length = nodeCommands.length;

            for (let i = 0; i < length; ++i) {
                nodeCommands[i].command.debugShowBoundingVolume = debugShowBoundingVolume;
            }
        }
    }

    function updateShadows(model) {
        if (model.shadows !== model._shadows) {
            model._shadows = model.shadows;

            let castShadows = Cesium.ShadowMode.castShadows(model.shadows);
            let receiveShadows = Cesium.ShadowMode.receiveShadows(model.shadows);
            let nodeCommands = model._nodeCommands;
            let length = nodeCommands.length;

            for (let i = 0; i < length; i++) {
                let nodeCommand = nodeCommands[i];
                nodeCommand.command.castShadows = castShadows;
                nodeCommand.command.receiveShadows = receiveShadows;
            }
        }
    }

    function getTranslucentRenderState(renderState) {
        let rs = Cesium.clone(renderState, true);
        rs.cull.enabled = false;
        rs.depthTest.enabled = true;
        rs.depthMask = false;
        rs.blending = Cesium.BlendingState.ALPHA_BLEND;

        return Cesium.RenderState.fromCache(rs);
    }

    function deriveTranslucentCommand(command) {
        let translucentCommand = Cesium.DrawCommand.shallowClone(command);
        translucentCommand.pass = Cesium.Pass.TRANSLUCENT;
        translucentCommand.renderState = getTranslucentRenderState(command.renderState);
        return translucentCommand;
    }

    function updateColor(model, frameState, forceDerive) {
        // Generate translucent commands when the blend color has an alpha in the range (0.0, 1.0) exclusive
        let scene3DOnly = frameState.scene3DOnly;
        let alpha = model.color.alpha;
        if ((alpha > 0.0) && (alpha < 1.0)) {
            let nodeCommands = model._nodeCommands;
            let length = nodeCommands.length;
            if (!Cesium.defined(nodeCommands[0].translucentCommand) || forceDerive) {
                for (let i = 0; i < length; ++i) {
                    let nodeCommand = nodeCommands[i];
                    let command = nodeCommand.command;
                    nodeCommand.translucentCommand = deriveTranslucentCommand(command);
                    if (!scene3DOnly) {
                        let command2D = nodeCommand.command2D;
                        nodeCommand.translucentCommand2D = deriveTranslucentCommand(command2D);
                    }
                }
            }
        }
    }

    function getProgramId(model, program) {
        let programs = model._rendererResources.programs;
        for (let id in programs) {
            if (programs.hasOwnProperty(id)) {
                if (programs[id] === program) {
                    return id;
                }
            }
        }
    }

    function createSilhouetteProgram(model, program, frameState) {
        let vs = program.vertexShaderSource.sources[0];
        let attributeLocations = program._attributeLocations;
        let normalAttributeName = model._normalAttributeName;

        // Modified from http://forum.unity3d.com/threads/toon-outline-but-with-diffuse-surface.24668/
        vs = Cesium.ShaderSource.replaceMain(vs, 'gltf_silhouette_main');
        vs +=
            'uniform float gltf_silhouetteSize; \n' +
            'void main() \n' +
            '{ \n' +
            '    gltf_silhouette_main(); \n' +
            '    vec3 n = normalize(czm_normal3D * ' + normalAttributeName + '); \n' +
            '    n.x *= czm_projection[0][0]; \n' +
            '    n.y *= czm_projection[1][1]; \n' +
            '    vec4 clip = gl_Position; \n' +
            '    clip.xy += n.xy * clip.w * gltf_silhouetteSize / czm_viewport.z; \n' +
            '    gl_Position = clip; \n' +
            '}';

        let fs =
            'uniform vec4 gltf_silhouetteColor; \n' +
            'void main() \n' +
            '{ \n' +
            '    gl_FragColor = czm_gammaCorrect(gltf_silhouetteColor); \n' +
            '}';

        return ShaderProgram.fromCache({
            context : frameState.context,
            vertexShaderSource : vs,
            fragmentShaderSource : fs,
            attributeLocations : attributeLocations
        });
    }

    function hasSilhouette(model, frameState) {
        return silhouetteSupported(frameState.context) && (model.silhouetteSize > 0.0) && (model.silhouetteColor.alpha > 0.0) && Cesium.defined(model._normalAttributeName);
    }

    function hasTranslucentCommands(model) {
        let nodeCommands = model._nodeCommands;
        let length = nodeCommands.length;
        for (let i = 0; i < length; ++i) {
            let nodeCommand = nodeCommands[i];
            let command = nodeCommand.command;
            if (command.pass === Cesium.Pass.TRANSLUCENT) {
                return true;
            }
        }
        return false;
    }

    function isTranslucent(model) {
        return (model.color.alpha > 0.0) && (model.color.alpha < 1.0);
    }

    function isInvisible(model) {
        return (model.color.alpha === 0.0);
    }

    function alphaDirty(currAlpha, prevAlpha) {
        // Returns whether the alpha state has changed between invisible, translucent, or opaque
        return (Math.floor(currAlpha) !== Math.floor(prevAlpha)) || (Math.ceil(currAlpha) !== Math.ceil(prevAlpha));
    }

    let silhouettesLength = 0;

    function createSilhouetteCommands(model, frameState) {
        // Wrap around after exceeding the 8-bit stencil limit.
        // The reference is unique to each model until this point.
        let stencilReference = (++silhouettesLength) % 255;

        // If the model is translucent the silhouette needs to be in the translucent pass.
        // Otherwise the silhouette would be rendered before the model.
        let silhouetteTranslucent = hasTranslucentCommands(model) || isTranslucent(model) || (model.silhouetteColor.alpha < 1.0);
        let silhouettePrograms = model._rendererResources.silhouettePrograms;
        let scene3DOnly = frameState.scene3DOnly;
        let nodeCommands = model._nodeCommands;
        let length = nodeCommands.length;
        for (let i = 0; i < length; ++i) {
            let nodeCommand = nodeCommands[i];
            let command = nodeCommand.command;

            // Create model command
            let modelCommand = isTranslucent(model) ? nodeCommand.translucentCommand : command;
            let silhouetteModelCommand = DrawCommand.shallowClone(modelCommand);
            let renderState = Cesium.clone(modelCommand.renderState);

            // Write the reference value into the stencil buffer.
            renderState.stencilTest = {
                enabled : true,
                frontFunction : Cesium.WebGLConstants.ALWAYS,
                backFunction : Cesium.WebGLConstants.ALWAYS,
                reference : stencilReference,
                mask : ~0,
                frontOperation : {
                    fail : Cesium.WebGLConstants.KEEP,
                    zFail : Cesium.WebGLConstants.KEEP,
                    zPass : Cesium.WebGLConstants.REPLACE
                },
                backOperation : {
                    fail : Cesium.WebGLConstants.KEEP,
                    zFail : Cesium.WebGLConstants.KEEP,
                    zPass : Cesium.WebGLConstants.REPLACE
                }
            };

            if (isInvisible(model)) {
                // When the model is invisible disable color and depth writes but still write into the stencil buffer
                renderState.colorMask = {
                    red : false,
                    green : false,
                    blue : false,
                    alpha : false
                };
                renderState.depthMask = false;
            }
            renderState = Cesium.RenderState.fromCache(renderState);
            silhouetteModelCommand.renderState = renderState;
            nodeCommand.silhouetteModelCommand = silhouetteModelCommand;

            // Create color command
            let silhouetteColorCommand = Cesium.DrawCommand.shallowClone(command);
            renderState = Cesium.clone(command.renderState, true);
            renderState.depthTest.enabled = true;
            renderState.cull.enabled = false;
            if (silhouetteTranslucent) {
                silhouetteColorCommand.pass = Cesium.Pass.TRANSLUCENT;
                renderState.depthMask = false;
                renderState.blending = Cesium.BlendingState.ALPHA_BLEND;
            }

            // Only render silhouette if the value in the stencil buffer equals the reference
            renderState.stencilTest = {
                enabled : true,
                frontFunction : Cesium.WebGLConstants.NOTEQUAL,
                backFunction : Cesium.WebGLConstants.NOTEQUAL,
                reference : stencilReference,
                mask : ~0,
                frontOperation : {
                    fail : Cesium.WebGLConstants.KEEP,
                    zFail : Cesium.WebGLConstants.KEEP,
                    zPass : Cesium.WebGLConstants.KEEP
                },
                backOperation : {
                    fail : Cesium.WebGLConstants.KEEP,
                    zFail : Cesium.WebGLConstants.KEEP,
                    zPass : Cesium.WebGLConstants.KEEP
                }
            };
            renderState = Cesium.RenderState.fromCache(renderState);

            // If the silhouette program has already been cached use it
            let program = command.shaderProgram;
            let id = getProgramId(model, program);
            let silhouetteProgram = silhouettePrograms[id];
            if (!defined(silhouetteProgram)) {
                silhouetteProgram = createSilhouetteProgram(model, program, frameState);
                silhouettePrograms[id] = silhouetteProgram;
            }

            let silhouetteUniformMap = Cesium.combine(command.uniformMap, {
                gltf_silhouetteColor : createSilhouetteColorFunction(model),
                gltf_silhouetteSize : createSilhouetteSizeFunction(model)
            });

            silhouetteColorCommand.renderState = renderState;
            silhouetteColorCommand.shaderProgram = silhouetteProgram;
            silhouetteColorCommand.uniformMap = silhouetteUniformMap;
            silhouetteColorCommand.castShadows = false;
            silhouetteColorCommand.receiveShadows = false;
            nodeCommand.silhouetteColorCommand = silhouetteColorCommand;

            if (!scene3DOnly) {
                let command2D = nodeCommand.command2D;
                let silhouetteModelCommand2D = Cesium.DrawCommand.shallowClone(silhouetteModelCommand);
                silhouetteModelCommand2D.boundingVolume = command2D.boundingVolume;
                silhouetteModelCommand2D.modelMatrix = command2D.modelMatrix;
                nodeCommand.silhouetteModelCommand2D = silhouetteModelCommand2D;

                let silhouetteColorCommand2D = Cesium.DrawCommand.shallowClone(silhouetteColorCommand);
                silhouetteModelCommand2D.boundingVolume = command2D.boundingVolume;
                silhouetteModelCommand2D.modelMatrix = command2D.modelMatrix;
                nodeCommand.silhouetteColorCommand2D = silhouetteColorCommand2D;
            }
        }
    }

    function modifyShaderForClippingPlanes(shader, clippingPlaneCollection, context) {
        shader = Cesium.ShaderSource.replaceMain(shader, 'gltf_clip_main');
        shader += Cesium.Model._getClippingFunction(clippingPlaneCollection, context) + '\n';
        shader +=
            'uniform sampler2D gltf_clippingPlanes; \n' +
            'uniform mat4 gltf_clippingPlanesMatrix; \n' +
            'uniform vec4 gltf_clippingPlanesEdgeStyle; \n' +
            'void main() \n' +
            '{ \n' +
            '    gltf_clip_main(); \n' +
            getClipAndStyleCode('gltf_clippingPlanes', 'gltf_clippingPlanesMatrix', 'gltf_clippingPlanesEdgeStyle') +
            '} \n';
        return shader;
    }

    function updateSilhouette(model, frameState, force) {
        // Generate silhouette commands when the silhouette size is greater than 0.0 and the alpha is greater than 0.0
        // There are two silhouette commands:
        //     1. silhouetteModelCommand : render model normally while enabling stencil mask
        //     2. silhouetteColorCommand : render enlarged model with a solid color while enabling stencil tests
        if (!hasSilhouette(model, frameState)) {
            return;
        }

        let nodeCommands = model._nodeCommands;
        let dirty = alphaDirty(model.color.alpha, model._colorPreviousAlpha) ||
                    alphaDirty(model.silhouetteColor.alpha, model._silhouetteColorPreviousAlpha) ||
                    !defined(nodeCommands[0].silhouetteModelCommand);

        model._colorPreviousAlpha = model.color.alpha;
        model._silhouetteColorPreviousAlpha = model.silhouetteColor.alpha;

        if (dirty || force) {
            createSilhouetteCommands(model, frameState);
        }
    }

    function updateClippingPlanes(model, frameState) {
        let clippingPlanes = model._clippingPlanes;
        if (Cesium.defined(clippingPlanes) && clippingPlanes.owner === model) {
            if (clippingPlanes.enabled) {
                clippingPlanes.update(frameState);
            }
        }
    }

    let scratchBoundingSphere = new Cesium.BoundingSphere();

    function scaleInPixels(positionWC, radius, frameState) {
        scratchBoundingSphere.center = positionWC;
        scratchBoundingSphere.radius = radius;
        return frameState.camera.getPixelSize(scratchBoundingSphere, frameState.context.drawingBufferWidth, frameState.context.drawingBufferHeight);
    }

    let scratchPosition = new Cesium.Cartesian3();
    let scratchCartographic = new Cesium.Cartographic();

    function getScale(model, frameState) {
        let scale = model.scale;

        if (model.minimumPixelSize !== 0.0) {
            // Compute size of bounding sphere in pixels
            let context = frameState.context;
            let maxPixelSize = Math.max(context.drawingBufferWidth, context.drawingBufferHeight);
            let m = Cesium.defined(model._clampedModelMatrix) ? model._clampedModelMatrix : model.modelMatrix;
            scratchPosition.x = m[12];
            scratchPosition.y = m[13];
            scratchPosition.z = m[14];

            if (Cesium.defined(model._rtcCenter)) {
                Cesium.Cartesian3.add(model._rtcCenter, scratchPosition, scratchPosition);
            }

            if (model._mode !== Cesium.SceneMode.SCENE3D) {
                let projection = frameState.mapProjection;
                let cartographic = projection.ellipsoid.cartesianToCartographic(scratchPosition, scratchCartographic);
                projection.project(cartographic, scratchPosition);
                Cartesian3.fromElements(scratchPosition.z, scratchPosition.x, scratchPosition.y, scratchPosition);
            }

            let radius = model.boundingSphere.radius;
            let metersPerPixel = scaleInPixels(scratchPosition, radius, frameState);

            // metersPerPixel is always > 0.0
            let pixelsPerMeter = 1.0 / metersPerPixel;
            let diameterInPixels = Math.min(pixelsPerMeter * (2.0 * radius), maxPixelSize);

            // Maintain model's minimum pixel size
            if (diameterInPixels < model.minimumPixelSize) {
                scale = (model.minimumPixelSize * metersPerPixel) / (2.0 * model._initialRadius);
            }
        }

        return Cesium.defined(model.maximumScale) ? Math.min(model.maximumScale, scale) : scale;
    }

    function releaseCachedGltf(model) {
        if (Cesium.defined(model._cacheKey) && Cesium.defined(model._cachedGltf) && (--model._cachedGltf.count === 0)) {
            delete gltfCache[model._cacheKey];
        }
        model._cachedGltf = undefined;
    }

    ///////////////////////////////////////////////////////////////////////////

    // function CachedRendererResources(context, cacheKey) {
    //     this.buffers = undefined;
    //     this.vertexArrays = undefined;
    //     this.programs = undefined;
    //     this.sourceShaders = undefined;
    //     this.silhouettePrograms = undefined;
    //     this.textures = undefined;
    //     this.samplers = undefined;
    //     this.renderStates = undefined;
    //     this.ready = false;

    //     this.context = context;
    //     this.cacheKey = cacheKey;
    //     this.count = 0;
    // }

    // function destroy(property) {
    //     for (let name in property) {
    //         if (property.hasOwnProperty(name)) {
    //             property[name].destroy();
    //         }
    //     }
    // }

    // function destroyCachedRendererResources(resources) {
    //     destroy(resources.buffers);
    //     destroy(resources.vertexArrays);
    //     destroy(resources.programs);
    //     destroy(resources.silhouettePrograms);
    //     destroy(resources.textures);
    // }

    // CachedRendererResources.prototype.release = function() {
    //     if (--this.count === 0) {
    //         if (Cesium.defined(this.cacheKey)) {
    //             // Remove if this was cached
    //             delete this.context.cache.modelRendererResourceCache[this.cacheKey];
    //         }
    //         destroyCachedRendererResources(this);
    //         return destroyObject(this);
    //     }

    //     return undefined;
    // };

    ///////////////////////////////////////////////////////////////////////////

    function getUpdateHeightCallback(model, ellipsoid, cartoPosition) {
        return function(clampedPosition) {
            if (model.heightReference === Cesium.HeightReference.RELATIVE_TO_GROUND) {
                let clampedCart = ellipsoid.cartesianToCartographic(clampedPosition, scratchCartographic);
                clampedCart.height += cartoPosition.height;
                ellipsoid.cartographicToCartesian(clampedCart, clampedPosition);
            }

            let clampedModelMatrix = model._clampedModelMatrix;

            // Modify clamped model matrix to use new height
            Cesium.Matrix4.clone(model.modelMatrix, clampedModelMatrix);
            clampedModelMatrix[12] = clampedPosition.x;
            clampedModelMatrix[13] = clampedPosition.y;
            clampedModelMatrix[14] = clampedPosition.z;

            model._heightChanged = true;
        };
    }

    function updateClamping(model) {
        if (Cesium.defined(model._removeUpdateHeightCallback)) {
            model._removeUpdateHeightCallback();
            model._removeUpdateHeightCallback = undefined;
        }

        let scene = model._scene;
        if (!Cesium.defined(scene) || !Cesium.defined(scene.globe) || (model.heightReference === HeightReference.NONE)) {
            //>>includeStart('debug', pragmas.debug);
            if (model.heightReference !== Cesium.HeightReference.NONE) {
                throw new Cesium.DeveloperError('Height reference is not supported without a scene and globe.');
            }
            //>>includeEnd('debug');
            model._clampedModelMatrix = undefined;
            return;
        }

        let globe = scene.globe;
        let ellipsoid = globe.ellipsoid;

        // Compute cartographic position so we don't recompute every update
        let modelMatrix = model.modelMatrix;
        scratchPosition.x = modelMatrix[12];
        scratchPosition.y = modelMatrix[13];
        scratchPosition.z = modelMatrix[14];
        let cartoPosition = ellipsoid.cartesianToCartographic(scratchPosition);

        if (!Cesium.defined(model._clampedModelMatrix)) {
            model._clampedModelMatrix = Matrix4.clone(modelMatrix, new Cesium.Matrix4());
        }

        // Install callback to handle updating of terrain tiles
        let surface = globe._surface;
        model._removeUpdateHeightCallback = surface.updateHeight(cartoPosition, getUpdateHeightCallback(model, ellipsoid, cartoPosition));

        // Set the correct height now
        let height = globe.getHeight(cartoPosition);
        if (Cesium.defined(height)) {
            // Get callback with cartoPosition being the non-clamped position
            let cb = getUpdateHeightCallback(model, ellipsoid, cartoPosition);

            // Compute the clamped cartesian and call updateHeight callback
            Cartographic.clone(cartoPosition, scratchCartographic);
            scratchCartographic.height = height;
            ellipsoid.cartographicToCartesian(scratchCartographic, scratchPosition);
            cb(scratchPosition);
        }
    }

    let scratchDisplayConditionCartesian = new Cesium.Cartesian3();
    let scratchDistanceDisplayConditionCartographic = new Cesium.Cartographic();

    function distanceDisplayConditionVisible(model, frameState) {
        let distance2;
        let ddc = model.distanceDisplayCondition;
        let nearSquared = ddc.near * ddc.near;
        let farSquared = ddc.far * ddc.far;

        if (frameState.mode === Cesium.SceneMode.SCENE2D) {
            let frustum2DWidth = frameState.camera.frustum.right - frameState.camera.frustum.left;
            distance2 = frustum2DWidth * 0.5;
            distance2 = distance2 * distance2;
        } else {
            // Distance to center of primitive's reference frame
            let position = Cesium.Matrix4.getTranslation(model.modelMatrix, scratchDisplayConditionCartesian);
            if (frameState.mode === Cesium.SceneMode.COLUMBUS_VIEW) {
                let projection = frameState.mapProjection;
                let ellipsoid = projection.ellipsoid;
                let cartographic = ellipsoid.cartesianToCartographic(position, scratchDistanceDisplayConditionCartographic);
                position = projection.project(cartographic, position);
                Cesium.Cartesian3.fromElements(position.z, position.x, position.y, position);
            }
            distance2 = Cesium.Cartesian3.distanceSquared(position, frameState.camera.positionWC);
        }

        return (distance2 >= nearSquared) && (distance2 <= farSquared);
    }

    M3DModelParser.prototype.update = function(frameState) {
        if (frameState.mode === Cesium.SceneMode.MORPHING) {
            return;
        }

        if (!Cesium.FeatureDetection.supportsWebP.initialized) {
            Cesium.FeatureDetection.supportsWebP.initialize();
            return;
        }
        let supportsWebP = Cesium.FeatureDetection.supportsWebP();

        let context = frameState.context;
        this._defaultTexture = context.defaultTexture;

        if ((this._state === ModelState.NEEDS_LOAD) && Cesium.defined(this.gltf)) {
            // Use renderer resources from cache instead of loading/creating them?
            let cachedRendererResources;
            let cacheKey = this.cacheKey;
            if (Cesium.defined(cacheKey)) { // cache key given? this model will pull from or contribute to context level cache
                context.cache.modelRendererResourceCache = Cesium.defaultValue(context.cache.modelRendererResourceCache, {});
                let modelCaches = context.cache.modelRendererResourceCache;

                cachedRendererResources = modelCaches[this.cacheKey];
                if (Cesium.defined(cachedRendererResources)) {
                    if (!cachedRendererResources.ready) {
                        // Cached resources for the model are not loaded yet.  We'll
                        // try again every frame until they are.
                        return;
                    }

                    ++cachedRendererResources.count;
                    this._loadRendererResourcesFromCache = true;
                } else {
                    cachedRendererResources = new CachedRendererResources(context, cacheKey);
                    cachedRendererResources.count = 1;
                    modelCaches[this.cacheKey] = cachedRendererResources;
                }
                this._cachedRendererResources = cachedRendererResources;
            } else { // cache key not given? this model doesn't care about context level cache at all. Cache is here to simplify freeing on destroy.
                cachedRendererResources = new CachedRendererResources(context);
                cachedRendererResources.count = 1;
                this._cachedRendererResources = cachedRendererResources;
            }

            this._state = ModelState.LOADING;
            if (this._state !== ModelState.FAILED) {
                let extensions = this.gltf.extensions;
                if (Cesium.defined(extensions) && Cesium.defined(extensions.CESIUM_RTC)) {
                    let center = Cesium.Cartesian3.fromArray(extensions.CESIUM_RTC.center);
                    if (!Cesium.Cartesian3.equals(center, Cesium.Cartesian3.ZERO)) {
                        this._rtcCenter3D = center;

                        let projection = frameState.mapProjection;
                        let ellipsoid = projection.ellipsoid;
                        let cartographic = ellipsoid.cartesianToCartographic(this._rtcCenter3D);
                        let projectedCart = projection.project(cartographic);
                        Cesium.Cartesian3.fromElements(projectedCart.z, projectedCart.x, projectedCart.y, projectedCart);
                        this._rtcCenter2D = projectedCart;

                        this._rtcCenterEye = new Cesium.Cartesian3();
                        this._rtcCenter = this._rtcCenter3D;
                    }
                }

                Cesium.addPipelineExtras(this.gltf);

                this._loadResources = new Cesium.ModelLoadResources();
                if (!this._loadRendererResourcesFromCache) {
                    // Buffers are required to updateVersion
                    Cesium.ModelUtility.parseBuffers(this, bufferLoad);
                }
            }
        }

        let loadResources = this._loadResources;
        let incrementallyLoadTextures = this._incrementallyLoadTextures;
        let justLoaded = false;

        if (this._state === ModelState.LOADING) {
            // Transition from LOADING -> LOADED once resources are downloaded and created.
            // Textures may continue to stream in while in the LOADED state.
            if (loadResources.pendingBufferLoads === 0) {
                if (!loadResources.initialized) {
                    frameState.brdfLutGenerator.update(frameState);

                    Cesium.ModelUtility.checkSupportedExtensions(this.extensionsRequired, supportsWebP);
                    Cesium.ModelUtility.updateForwardAxis(this);

                    // glTF pipeline updates, not needed if loading from cache
                    if (!Cesium.defined(this.gltf.extras.sourceVersion)) {
                        let gltf = this.gltf;
                        // Add the original version so it remains cached
                        gltf.extras.sourceVersion = Cesium.ModelUtility.getAssetVersion(gltf);
                        gltf.extras.sourceKHRTechniquesWebGL = Cesium.defined(Cesium.ModelUtility.getUsedExtensions(gltf).KHR_techniques_webgl);

                        this._sourceVersion = gltf.extras.sourceVersion;
                        this._sourceKHRTechniquesWebGL = gltf.extras.sourceKHRTechniquesWebGL;

                        Cesium.updateVersion(gltf);
                        Cesium.addDefaults(gltf);

                        let options = {
                            addBatchIdToGeneratedShaders: this._addBatchIdToGeneratedShaders,
                            // qwk --------
                            u_height: this.u_height,
                            u_isFlatten: this.u_isFlatten,
                            u_arrayLength: this.u_arrayLength,
                            u_positionArray: this.u_positionArray,
                            //zlf --------
                            u_isAttributeFilter: this.u_isAttributeFilter,
                            u_offset: this.u_offset,
                            u_pickTextureCoord:this.u_pickTextureCoord
                        };

                        Cesium.processModelMaterialsCommon(gltf, options);
                        Cesium.processPbrMaterials(gltf, options);
                    }

                    this._sourceVersion = this.gltf.extras.sourceVersion;
                    this._sourceKHRTechniquesWebGL = this.gltf.extras.sourceKHRTechniquesWebGL;

                    // Skip dequantizing in the shader if not encoded
                    this._dequantizeInShader = this._dequantizeInShader && Cesium.DracoLoader.hasExtension(this);

                    // We do this after to make sure that the ids don't change
                    addBuffersToLoadResources(this);
                    parseArticulations(this);
                    parseTechniques(this);
                    if (!this._loadRendererResourcesFromCache) {
                        parseBufferViews(this);
                        parseShaders(this);
                        parsePrograms(this);
                        parseTextures(this, context, supportsWebP);
                    }
                    parseMaterials(this);
                    parseMeshes(this);
                    parseNodes(this);

                    // Start draco decoding
                    Cesium.DracoLoader.parse(this, context);

                    loadResources.initialized = true;
                }

                if (!loadResources.finishedDecoding()) {
                    Cesium.DracoLoader.decodeModel(this, context)
                        .otherwise(Cesium.ModelUtility.getFailedLoadFunction(this, 'model', this.basePath));
                }

                if (loadResources.finishedDecoding() && !loadResources.resourcesParsed) {
                    this._boundingSphere = Cesium.ModelUtility.computeBoundingSphere(this);
                    this._initialRadius = this._boundingSphere.radius;

                    Cesium.DracoLoader.cacheDataForModel(this);

                    loadResources.resourcesParsed = true;
                }

                if (loadResources.resourcesParsed &&
                    loadResources.pendingShaderLoads === 0) {
                    createResources(this, frameState);
                }
            }

            if (loadResources.finished() ||
                (incrementallyLoadTextures && loadResources.finishedEverythingButTextureCreation())) {
                this._state = ModelState.LOADED;
                justLoaded = true;
            }
        }

        // Incrementally stream textures.
        if (Cesium.defined(loadResources) && (this._state === ModelState.LOADED)) {
            if (incrementallyLoadTextures && !justLoaded) {
                createResources(this, frameState);
            }

            if (loadResources.finished()) {
                this._loadResources = undefined;  // Clear CPU memory since WebGL resources were created.

                let resources = this._rendererResources;
                let cachedResources = this._cachedRendererResources;

                cachedResources.buffers = resources.buffers;
                cachedResources.vertexArrays = resources.vertexArrays;
                cachedResources.programs = resources.programs;
                cachedResources.sourceShaders = resources.sourceShaders;
                cachedResources.silhouettePrograms = resources.silhouettePrograms;
                cachedResources.textures = resources.textures;
                cachedResources.samplers = resources.samplers;
                cachedResources.renderStates = resources.renderStates;
                cachedResources.ready = true;

                // The normal attribute name is required for silhouettes, so get it before the gltf JSON is released
                this._normalAttributeName = Cesium.ModelUtility.getAttributeOrUniformBySemantic(this.gltf, 'NORMAL');

                // Vertex arrays are unique to this model, do not store in cache.
                if (Cesium.defined(this._precreatedAttributes)) {
                    cachedResources.vertexArrays = {};
                }

                if (this.releaseGltfJson) {
                    releaseCachedGltf(this);
                }
            }
        }

        let iblSupported = Cesium.OctahedralProjectedCubeMap.isSupported(context);
        if (this._shouldUpdateSpecularMapAtlas && iblSupported) {
            this._shouldUpdateSpecularMapAtlas = false;
            this._specularEnvironmentMapAtlas = this._specularEnvironmentMapAtlas && this._specularEnvironmentMapAtlas.destroy();
            this._specularEnvironmentMapAtlas = undefined;
            if (Cesium.defined(this._specularEnvironmentMaps)) {
                this._specularEnvironmentMapAtlas = new Cesium.OctahedralProjectedCubeMap(this._specularEnvironmentMaps);
                let that = this;
                this._specularEnvironmentMapAtlas.readyPromise.then(function() {
                    that._shouldRegenerateShaders = true;
                });
            }

            // Regenerate shaders to not use an environment map. Will be set to true again if there was a new environment map and it is ready.
            this._shouldRegenerateShaders = true;
        }

        if (Cesium.defined(this._specularEnvironmentMapAtlas)) {
            this._specularEnvironmentMapAtlas.update(frameState);
        }

        let recompileWithDefaultAtlas = !Cesium.defined(this._specularEnvironmentMapAtlas) && Cesium.defined(frameState.specularEnvironmentMaps) && !this._useDefaultSpecularMaps;
        let recompileWithoutDefaultAtlas = !Cesium.defined(frameState.specularEnvironmentMaps) && this._useDefaultSpecularMaps;

        let recompileWithDefaultSHCoeffs = !Cesium.defined(this._sphericalHarmonicCoefficients) && Cesium.defined(frameState.sphericalHarmonicCoefficients) && !this._useDefaultSphericalHarmonics;
        let recompileWithoutDefaultSHCoeffs = !Cesium.defined(frameState.sphericalHarmonicCoefficients) && this._useDefaultSphericalHarmonics;

        this._shouldRegenerateShaders = this._shouldRegenerateShaders || recompileWithDefaultAtlas || recompileWithoutDefaultAtlas || recompileWithDefaultSHCoeffs || recompileWithoutDefaultSHCoeffs;

        this._useDefaultSpecularMaps = !Cesium.defined(this._specularEnvironmentMapAtlas) && Cesium.defined(frameState.specularEnvironmentMaps);
        this._useDefaultSphericalHarmonics = !Cesium.defined(this._sphericalHarmonicCoefficients) && Cesium.defined(frameState.sphericalHarmonicCoefficients);

        let silhouette = hasSilhouette(this, frameState);
        let translucent = isTranslucent(this);
        let invisible = isInvisible(this);
        let displayConditionPassed = Cesium.defined(this.distanceDisplayCondition) ? distanceDisplayConditionVisible(this, frameState) : true;
        let show = this.show && displayConditionPassed && (this.scale !== 0.0) && (!invisible || silhouette);

        if ((show && this._state === ModelState.LOADED) || justLoaded) {
            let animated = this.activeAnimations.update(frameState) || this._cesiumAnimationsDirty;
            this._cesiumAnimationsDirty = false;
            this._dirty = false;
            let modelMatrix = this.modelMatrix;

            let modeChanged = frameState.mode !== this._mode;
            this._mode = frameState.mode;

            // Model's model matrix needs to be updated
            let modelTransformChanged = !Cesium.Matrix4.equals(this._modelMatrix, modelMatrix) ||
                (this._scale !== this.scale) ||
                (this._minimumPixelSize !== this.minimumPixelSize) || (this.minimumPixelSize !== 0.0) || // Minimum pixel size changed or is enabled
                (this._maximumScale !== this.maximumScale) ||
                (this._heightReference !== this.heightReference) || this._heightChanged ||
                modeChanged;

            if (modelTransformChanged || justLoaded) {
                Cesium.Matrix4.clone(modelMatrix, this._modelMatrix);

                updateClamping(this);

                if (Cesium.defined(this._clampedModelMatrix)) {
                    modelMatrix = this._clampedModelMatrix;
                }

                this._scale = this.scale;
                this._minimumPixelSize = this.minimumPixelSize;
                this._maximumScale = this.maximumScale;
                this._heightReference = this.heightReference;
                this._heightChanged = false;

                let scale = getScale(this, frameState);
                let computedModelMatrix = this._computedModelMatrix;
                Cesium.Matrix4.multiplyByUniformScale(modelMatrix, scale, computedModelMatrix);
                if (this._upAxis === Cesium.Axis.Y) {
                    Cesium.Matrix4.multiplyTransformation(computedModelMatrix, Cesium.Axis.Y_UP_TO_Z_UP, computedModelMatrix);
                } else if (this._upAxis === Cesium.Axis.X) {
                    Cesium.Matrix4.multiplyTransformation(computedModelMatrix, Cesium.Axis.X_UP_TO_Z_UP, computedModelMatrix);
                }
                if (this.forwardAxis === Cesium.Axis.Z) {
                    // glTF 2.0 has a Z-forward convention that must be adapted here to X-forward.
                    Cesium.Matrix4.multiplyTransformation(computedModelMatrix, Cesium.Axis.Z_UP_TO_X_UP, computedModelMatrix);
                }
            }

            // Update modelMatrix throughout the graph as needed
            if (animated || modelTransformChanged || justLoaded) {
                updateNodeHierarchyModelMatrix(this, modelTransformChanged, justLoaded, frameState.mapProjection);
                this._dirty = true;

                if (animated || justLoaded) {
                    // Apply skins if animation changed any node transforms
                    applySkins(this);
                }
            }

            if (this._perNodeShowDirty) {
                this._perNodeShowDirty = false;
                updatePerNodeShow(this);
            }
            updatePickIds(this, context);
            updateWireframe(this);
            updateShowBoundingVolume(this);
            updateShadows(this);
            updateClippingPlanes(this, frameState);

            // Regenerate shaders if ClippingPlaneCollection state changed or it was removed
            let clippingPlanes = this._clippingPlanes;
            let currentClippingPlanesState = 0;
            let useClippingPlanes = Cesium.defined(clippingPlanes) && clippingPlanes.enabled && clippingPlanes.length > 0;
            let usesSH = Cesium.defined(this._sphericalHarmonicCoefficients) || this._useDefaultSphericalHarmonics;
            let usesSM = (Cesium.defined(this._specularEnvironmentMapAtlas) && this._specularEnvironmentMapAtlas.ready) || this._useDefaultSpecularMaps;
            if (useClippingPlanes || usesSH || usesSM) {
                let clippingPlanesOriginMatrix = Cesium.defaultValue(this.clippingPlanesOriginMatrix, modelMatrix);
                Cesium.Matrix4.multiply(context.uniformState.view3D, clippingPlanesOriginMatrix, this._clippingPlaneModelViewMatrix);
            }

            if (useClippingPlanes) {
                currentClippingPlanesState = clippingPlanes.clippingPlanesState;
            }

            let shouldRegenerateShaders = this._shouldRegenerateShaders;
            shouldRegenerateShaders = shouldRegenerateShaders || this._clippingPlanesState !== currentClippingPlanesState;
            this._clippingPlanesState = currentClippingPlanesState;

            // Regenerate shaders if color shading changed from last update
            let currentlyColorShadingEnabled = isColorShadingEnabled(this);
            if (currentlyColorShadingEnabled !== this._colorShadingEnabled) {
                this._colorShadingEnabled = currentlyColorShadingEnabled;
                shouldRegenerateShaders = true;
            }

            if (shouldRegenerateShaders) {
                regenerateShaders(this, frameState);
            } else {
                updateColor(this, frameState, false);
                updateSilhouette(this, frameState, false);
            }
        }

        if (justLoaded) {
            // Called after modelMatrix update.
            let model = this;
            frameState.afterRender.push(function() {
                model._ready = true;
                model._readyPromise.resolve(model);
            });
            return;
        }

        // We don't check show at the top of the function since we
        // want to be able to progressively load models when they are not shown,
        // and then have them visible immediately when show is set to true.
        if (show && !this._ignoreCommands) {
            // PERFORMANCE_IDEA: This is terrible
            let commandList = frameState.commandList;
            let passes = frameState.passes;
            let nodeCommands = this._nodeCommands;
            let length = nodeCommands.length;
            let i;
            let nc;

            let idl2D = frameState.mapProjection.ellipsoid.maximumRadius * Cesium.Math.PI;
            let boundingVolume;

            if (passes.render || (passes.pick && this.allowPicking)) {
                for (i = 0; i < length; ++i) {
                    nc = nodeCommands[i];
                    if (nc.show) {
                        let command = translucent ? nc.translucentCommand : nc.command;
                        command = silhouette ? nc.silhouetteModelCommand : command;
                        commandList.push(command);
                        boundingVolume = nc.command.boundingVolume;
                        if (frameState.mode === Cesium.SceneMode.SCENE2D &&
                            (boundingVolume.center.y + boundingVolume.radius > idl2D || boundingVolume.center.y - boundingVolume.radius < idl2D)) {
                            let command2D = translucent ? nc.translucentCommand2D : nc.command2D;
                            command2D = silhouette ? nc.silhouetteModelCommand2D : command2D;
                            commandList.push(command2D);
                        }
                    }
                }

                if (silhouette && !passes.pick) {
                    // Render second silhouette pass
                    for (i = 0; i < length; ++i) {
                        nc = nodeCommands[i];
                        if (nc.show) {
                            commandList.push(nc.silhouetteColorCommand);
                            boundingVolume = nc.command.boundingVolume;
                            if (frameState.mode === Cesium.SceneMode.SCENE2D &&
                                (boundingVolume.center.y + boundingVolume.radius > idl2D || boundingVolume.center.y - boundingVolume.radius < idl2D)) {
                                commandList.push(nc.silhouetteColorCommand2D);
                            }
                        }
                    }
                }
            }
        }
    };

    function destroyIfNotCached(rendererResources, cachedRendererResources) {
        if (rendererResources.programs !== cachedRendererResources.programs) {
            destroy(rendererResources.programs);
        }
        if (rendererResources.silhouettePrograms !== cachedRendererResources.silhouettePrograms) {
            destroy(rendererResources.silhouettePrograms);
        }
    }

    // Run from update iff:
    // - everything is loaded
    // - clipping planes state change OR color state set
    // Run this from destructor after removing color state and clipping plane state
    function regenerateShaders(model, frameState) {
        // In regards to _cachedRendererResources:
        // Fair to assume that this is data that should just never get modified due to clipping planes or model color.
        // So if clipping planes or model color active:
        // - delink _rendererResources.*programs and create new dictionaries.
        // - do NOT destroy any programs - might be used by copies of the model or by might be needed in the future if clipping planes/model color is deactivated

        // If clipping planes and model color inactive:
        // - destroy _rendererResources.*programs
        // - relink _rendererResources.*programs to _cachedRendererResources

        // In both cases, need to mark commands as dirty, re-run derived commands (elsewhere)

        let rendererResources = model._rendererResources;
        let cachedRendererResources = model._cachedRendererResources;
        destroyIfNotCached(rendererResources, cachedRendererResources);

        let programId;
        if (isClippingEnabled(model) || isColorShadingEnabled(model) || model._shouldRegenerateShaders) {
            model._shouldRegenerateShaders = false;

            rendererResources.programs = {};
            rendererResources.silhouettePrograms = {};

            let visitedPrograms = {};
            let techniques = model._sourceTechniques;
            let technique;

            for (let techniqueId in techniques) {
                if (techniques.hasOwnProperty(techniqueId)) {
                    technique = techniques[techniqueId];
                    programId = technique.program;
                    if (!visitedPrograms[programId]) {
                        visitedPrograms[programId] = true;
                        recreateProgram({
                            programId: programId,
                            techniqueId: techniqueId
                        }, model, frameState.context);
                    }
                }
            }
        } else {
            rendererResources.programs = cachedRendererResources.programs;
            rendererResources.silhouettePrograms = cachedRendererResources.silhouettePrograms;
        }

        // Fix all the commands, marking them as dirty so everything that derives will re-derive
        let rendererPrograms = rendererResources.programs;

        let nodeCommands = model._nodeCommands;
        let commandCount = nodeCommands.length;
        for (let i = 0; i < commandCount; ++i) {
            let nodeCommand = nodeCommands[i];
            programId = nodeCommand.programId;

            let renderProgram = rendererPrograms[programId];
            nodeCommand.command.shaderProgram = renderProgram;
            if (Cesium.defined(nodeCommand.command2D)) {
                nodeCommand.command2D.shaderProgram = renderProgram;
            }
        }

        // Force update silhouette commands/shaders
        updateColor(model, frameState, true);
        updateSilhouette(model, frameState, true);
    }

    /**
     * Returns true if this object was destroyed; otherwise, false.
     * <br /><br />
     * If this object was destroyed, it should not be used; calling any function other than
     * <code>isDestroyed</code> will result in a {@link DeveloperError} exception.
     *
     * @returns {Boolean} <code>true</code> if this object was destroyed; otherwise, <code>false</code>.
     *
     * @see Model#destroy
     */
    M3DModelParser.prototype.isDestroyed = function() {
        return false;
    };

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
     *
     * @example
     * model = model && model.destroy();
     *
     * @see Model#isDestroyed
     */
    M3DModelParser.prototype.destroy = function() {
        // Vertex arrays are unique to this model, destroy here.
        if (Cesium.defined(this._precreatedAttributes)) {
            destroy(this._rendererResources.vertexArrays);
        }

        if (Cesium.defined(this._removeUpdateHeightCallback)) {
            this._removeUpdateHeightCallback();
            this._removeUpdateHeightCallback = undefined;
        }

        if (Cesium.defined(this._terrainProviderChangedCallback)) {
            this._terrainProviderChangedCallback();
            this._terrainProviderChangedCallback = undefined;
        }

        // Shaders modified for clipping and for color don't get cached, so destroy these manually
        if (Cesium.defined(this._cachedRendererResources)) {
            destroyIfNotCached(this._rendererResources, this._cachedRendererResources);
        }

        this._rendererResources = undefined;
        this._cachedRendererResources = this._cachedRendererResources && this._cachedRendererResources.release();
        Cesium.DracoLoader.destroyCachedDataForModel(this);

        let pickIds = this._pickIds;
        let length = pickIds.length;
        for (let i = 0; i < length; ++i) {
            pickIds[i].destroy();
        }

        releaseCachedGltf(this);
        this._quantizedVertexShaders = undefined;

        // Only destroy the ClippingPlaneCollection if this is the owner - if this model is part of a Cesium3DTileset,
        // _clippingPlanes references a ClippingPlaneCollection that this model does not own.
        let clippingPlaneCollection = this._clippingPlanes;
        if (Cesium.defined(clippingPlaneCollection) && !clippingPlaneCollection.isDestroyed() && clippingPlaneCollection.owner === this) {
            clippingPlaneCollection.destroy();
        }
        this._clippingPlanes = undefined;

        this._specularEnvironmentMapAtlas = this._specularEnvironmentMapAtlas && this._specularEnvironmentMapAtlas.destroy();

        return destroyObject(this);
    };

    // exposed for testing
    M3DModelParser._getClippingFunction = Cesium.getClippingFunction;
    M3DModelParser._modifyShaderForColor = Cesium.modifyShaderForColor;

CesiumZondy.M3D.M3DModelParser = M3DModelParser;