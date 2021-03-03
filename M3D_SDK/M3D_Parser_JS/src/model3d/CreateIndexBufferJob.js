import { CesiumZondy } from '../core/Base';

function createIndexBuffer(bufferViewId, componentType, model, context) {
    let loadResources = model._loadResources;
    let bufferViews = model.gltf.bufferViews;
    let bufferView = bufferViews[bufferViewId];

    // Use bufferView created at runtime
    if (!Cesium.defined(bufferView)) {
        bufferView = loadResources.createdBufferViews[bufferViewId];
    }

    let indexBuffer = Cesium.Buffer.createIndexBuffer({
        context : context,
        typedArray : loadResources.getBuffer(bufferView),
        usage : Cesium.BufferUsage.STATIC_DRAW,
        indexDatatype : componentType
    });
    indexBuffer.vertexArrayDestroyable = false;
    model._rendererResources.buffers[bufferViewId] = indexBuffer;
    model._geometryByteLength += indexBuffer.sizeInBytes;
}

export default class CreateIndexBufferJob{
    constructor(){
        this.id = undefined;
        this.componentType = undefined;
        this.model = undefined;
        this.context = undefined;
    }
    set (id, componentType, model, context) {
        this.id = id;
        this.componentType = componentType;
        this.model = model;
        this.context = context;
    }

    execute() {
        createIndexBuffer(this.id, this.componentType, this.model, this.context);
    }
}
CesiumZondy.M3D.CreateIndexBufferJob = CreateIndexBufferJob;