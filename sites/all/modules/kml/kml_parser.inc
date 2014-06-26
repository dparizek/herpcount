<?php

/**
 * Parses a KML file and returns various properties.
 */
class KmlParser
{
    // kml file to parse
    public $kmlPath = '';

    // group points that are less than 'x' miles apart.
    public $maxDist = 5;

    // group points that are less than 'x' seconds apart.
    public $maxDelta = 600;

    // miles per hour to assign to lines without any speed data
    public $lineSpeed = 2;

    // returns array of arrays with distance
    // travelled and avg. speed in miles/mph/
    public function calcDistAndSpeed() {
	// parse points from kml
        $points = array();
        $xml = simplexml_load_file($this->kmlPath);

        $placeGroups = $this->calcPlaceGroups($xml);
        $lineGroups = $this->calcLineGroups($xml);
        $groups = array($placeGroups, $lineGroups);

        $allGroups = array(
            'groups' => array(),
            'waypoints' => 0
        ); 
        foreach ($groups as $group) {
            $allGroups['groups'] = array_merge($allGroups['groups'], $group['groups']);
            $allGroups['waypoints'] += $group['waypoints'];
        }

        return $allGroups;
    }

    // calculates distance and speed for lines
    protected function calcLineGroups($xml) {
        $groups = array();
        foreach ($xml->Placemark as $pm) {

            if ((!$pm->LineString) || (!$pm->LineString->coordinates)) {
                // no line data
                continue;
            }

            $ls = $pm->LineString;
            $lastPoint = false;
            $distance = 0;
            $coords = explode(' ', $ls->coordinates);
            foreach ($coords as $coord) {
                $point = explode(',', $coord);
                if ($lastPoint) {
                    $newDistance = $this->calcPointDist($lastPoint, $point);
                    if ($newDistance > $this->maxDist) {
                        // new group
                        array_push($groups, array(
                            'distance' => $distance,
                            'speed' => $this->lineSpeed));
                        $distance = 0;
                    } else {
                        $distance += $newDistance;
                    }
                }
                $lastPoint = $point;
            }

            // last group
            array_push($groups, array(
                'distance' => $distance,
                'speed' => $this->lineSpeed));
        }

        return array(
            'groups' => $groups,
            'waypoints' => 0
        );
    }

    // calculates distance and speed for Placemarks
    protected function calcPlaceGroups($xml) {
        $waypoints = 0;
	$points = array();
        foreach ($xml->Placemark as $pm) {
            if ((!$pm->Point) || !($pm->Point->coordinates)) {
                // no coordinate data present
                continue;
            }

            if (!$pm->TimeStamp) {
                // PMs without timestamps are treated as a single point!!
                $waypoints += 1;
                continue;
            }

            if ($pm->description) {
                // Ugh.... timestamp is incorrect in TrackMe files,
                // must ghetto parse correct timestamp from description field.
                if (preg_match('/<td>Time:\s+(.{19})</i', $pm->description, $matches)) {
                    $timestamp = strtotime($matches[1]);
                }
            }

            if (!$timestamp) {
                $timestamp = strtotime((string) $pm->TimeStamp->when[0]);
            }

            $coords = explode(',', $pm->Point->coordinates);
            $points[$timestamp] = $coords;
        }

        // order points by timestamp and add up the distances between them
        ksort($points);
        $groups = array();
        $distance = 0;
        foreach ($points as $key => $point) {
            if ($lastPoint) {
                // Distance between points in miles
                $newDist = $this->calcPointDist($lastPoint, $point);
                if (($newDist > $this->maxDist) || (($lastKey + $this->maxDelta) < $key)) {
                    // New group!
                    // Points are grouped together to avoid including distances
                    // that are between 2 different sessions.
                    if ($distance && ($firstKey != $lastKey)) {
                        // if distance == 0, or firstKey == lastKey,
                        // then the previous point was a singleton,
                        // and is not recorded.

                        // avgSpd is in mph
                        $avgSpd = $distance / (($lastKey - $firstKey) / 60 / 60);
                        array_push($groups, array(
                            'distance' => $distance,
                            'speed' => $avgSpd));
                    }
                    $distance = 0;
                    $firstKey = $key;
                } else {
                    // Add to existing group.
                    $distance += $newDist;
                }
            } else {
                $firstKey = $key;
            }

            $lastPoint = $point;
            $lastKey = $key;
        }

        // add last group
        if ($distance && ($firstKey != $lastKey)) {
            $avgSpd = $distance / ($lastKey - $firstKey) / 60;
            array_push($groups, array(
                'distance' => $distance,
                'speed' => $avgSpd));
        }

        return array(
            'groups' => $groups,
            'waypoints' => $waypoints
        );
    }

    // Calculates the distance between two points
    public function calcPointDist($point_1, $point_2) {
        // Algo from: http://www.poieditor.com/articles/how_to__calculate_the_distance_between_two_points/

        $source_lat = $point_1[0] * pi() / 180;
        $source_lon = $point_1[1] * pi() / 180;

        $dest_lat = $point_2[0] * pi() / 180;
        $dest_lon = $point_2[1] * pi() / 180;

        $R = 3958.75587; // earth's mean radius in miles

        $dLat  = $dest_lat - $source_lat;
        $dLong = $dest_lon - $source_lon;

        $a = sin($dLat/2) * sin($dLat/2) + cos($source_lat) * 
        cos($dest_lat) * sin($dLong/2) * sin($dLong/2);

        $b = 2 * atan2(sqrt($a), sqrt(1-$a));

        $distance = $R * $b;
        return $distance;
    }
}
