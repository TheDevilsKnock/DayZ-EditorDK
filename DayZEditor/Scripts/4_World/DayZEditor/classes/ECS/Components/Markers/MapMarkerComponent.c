/**
 * @class MapMarkerComponent
 * @brief Holds the reference to the 2D map marker associated with an entity.
 *
 * A MarkerUpdateSystem will use this component to update the marker's
 * position and visibility on the in-game map.
 */
class MapMarkerComponent : EcsComponent
{
    EditorObjectMapMarker m_Marker;

    void MapMarkerComponent(EditorObjectMapMarker marker)
    {
        m_Marker = marker;
    }
}