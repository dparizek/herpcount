/** Adds a KML file to a GMAP object. */
var showKml = function(gmap, kmlPath) {
    var kmlPaths = kmlPath;
    if (!(kmlPath instanceof Array)) {
        kmlPaths = [kmlPath];
    }

    for (var i = 0; i < kmlPaths.length; i++) {
        var geoxml = new GGeoXml(kmlPaths[i]);
        GEvent.addListener(geoxml, 'load', function() {
            gmap.setCenter(geoxml.getDefaultCenter());
        });
        gmap.addOverlay(geoxml);
    }
}

/** Call functions on GMAP only AFTER it has been inited. */
var initKmlMap = function(mapId, kmlPath) {
    $(document).ready(function () {
        if (GBrowserIsCompatible()) {
            var m = Drupal.gmap.getMap(mapId);
            if (m.map) {
                // Map is ready to go
                showKml(m.map, kmlPath);
            } else {
                m.bind('ready', function () {
                    showKml(m.map, kmlPath);
                });
            }
        }
    });
}
