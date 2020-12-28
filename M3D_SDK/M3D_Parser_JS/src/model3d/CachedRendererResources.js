import { CesiumZondy } from '../core/Base';

function destroy(property) {
    for (let name in property) {
        if (property.hasOwnProperty(name)) {
            property[name].destroy();
        }
    }
}

function destroyCachedRendererResources(resources) {
    destroy(resources.buffers);
    destroy(resources.vertexArrays);
    destroy(resources.programs);
    destroy(resources.silhouettePrograms);
    destroy(resources.textures);
}

export default class CachedRendererResources{
    constructor(context, cacheKey){
        this.buffers = undefined;
        this.vertexArrays = undefined;
        this.programs = undefined;
        this.sourceShaders = undefined;
        this.silhouettePrograms = undefined;
        this.textures = undefined;
        this.samplers = undefined;
        this.renderStates = undefined;
        this.ready = false;

        this.context = context;
        this.cacheKey = cacheKey;
        this.count = 0;
    }
}

CachedRendererResources.prototype.release = function() {
    if (--this.count === 0) {
        if (Cesium.defined(this.cacheKey)) {
            // Remove if this was cached
            delete this.context.cache.modelRendererResourceCache[this.cacheKey];
        }
        destroyCachedRendererResources(this);
        return Cesium.destroyObject(this);
    }

    return undefined;
};

CesiumZondy.M3D.CachedRendererResources = CachedRendererResources;

