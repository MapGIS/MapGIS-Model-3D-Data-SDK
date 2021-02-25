export default class MapGISM3DTileContent{
    constructor(tileset, tile, resource,arrayBuffer,byteOffset) {
        this._tileset = tileset;
        this._tile = tile;
        this._resource = resource;
        this._readyPromise = Cesium.when.defer();

        this.featurePropertiesDirty = false;

        this.initialize(this, arrayBuffer, byteOffset);
    }

    get featuresLength(){
        return 0;
    }
    get pointsLength(){
        return 0;
    }
    get trianglesLength(){
        return 0;
    }
    get geometryByteLength(){
        return 0;
    }
    get texturesByteLength(){
        return 0;
    }
    get batchTableByteLength(){
        return 0;
    }
    get innerContents(){
        return undefined;
    }
    get readyPromise() {
        return this._readyPromise.promise;
    }
    get tileset(){
        return this._tileset;
    }
    get tile(){
        return this._tile
    }
    get url(){
        return this._resource.getUrlComponent(true);
    }
    get batchTable(){
        return undefined;
    }

    initialize(content, arrayBuffer, byteOffset) {
        byteOffset = Cesium.defaultValue(byteOffset, 0);
        var uint8Array = new Uint8Array(arrayBuffer);
        var jsonString = Cesium.getStringFromTypedArray(uint8Array, byteOffset);
        var tilesetJson;

        try {
            tilesetJson = JSON.parse(jsonString);
        } catch (error) {
            content._readyPromise.reject(new Cesium.RuntimeError('Invalid tile content.'));
            return;
        }

        if (Cesium.defined(tilesetJson.childrenNode)) {
            var children = {boundingVolume:{geoBox:tilesetJson.childrenNode[0].geoBox }};
            Object.extend(tilesetJson.childrenNode[0], children);
            tilesetJson.children = tilesetJson.childrenNode;
         }

         Object.extend(tilesetJson, {
            boundingVolume: content._tile._header.boundingVolume,
            refine: 'REPLACE'
        });
        content._tileset.loadM3DTileSet(content._resource, tilesetJson, content._tile);
        content._readyPromise.resolve(content);
    }

    /**
     * Part of the {@link Cesium3DTileContent} interface.  <code>MapGISM3DTileContent</code>
     * always returns <code>false</code> since a tile of this type does not have any features.
     */
    hasProperty (batchId, name) {
        return false;
    }


    getFeature (batchId) {
        return undefined;
    }

    applyDebugSettings (enabled, color) {
    }

    applyStyle (style) {
    }

    update (tileset, frameState) {
    }

    isDestroyed () {
        return false;
    }

    destroy () {
        return Cesium.destroyObject(this);
    }
}
CesiumZondy.M3D.MapGISM3DTileContent = MapGISM3DTileContent;

