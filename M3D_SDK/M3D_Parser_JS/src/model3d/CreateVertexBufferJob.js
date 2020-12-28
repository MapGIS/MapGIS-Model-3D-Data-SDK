import { CesiumZondy } from '../core/Base';

function createVertexBuffer(bufferViewId, model, context) {
        let loadResources = model._loadResources;
        let bufferViews = model.gltf.bufferViews;
        let bufferView = bufferViews[bufferViewId];
    
        // Use bufferView created at runtime
        if (!Cesium.defined(bufferView)) {
            bufferView = loadResources.createdBufferViews[bufferViewId];
        }
    
        let vertexBuffer = Cesium.Buffer.createVertexBuffer({
            context : context,
            typedArray : loadResources.getBuffer(bufferView),
            usage : Cesium.BufferUsage.STATIC_DRAW
        });
        vertexBuffer.vertexArrayDestroyable = false;
        model._rendererResources.buffers[bufferViewId] = vertexBuffer;
        model._geometryByteLength += vertexBuffer.sizeInBytes;
    };

export default class CreateVertexBufferJob{
    constructor(){
        this.id = undefined;
        this.model = undefined;
        this.context = undefined;
    }

    set(id, model, context){
        this.id = id;
        this.model = model;
        this.context = context;
    }

    execute() {
        createVertexBuffer(this.id, this.model, this.context);
    }
}

CesiumZondy.M3D.CreateVertexBufferJob = CreateVertexBufferJob;