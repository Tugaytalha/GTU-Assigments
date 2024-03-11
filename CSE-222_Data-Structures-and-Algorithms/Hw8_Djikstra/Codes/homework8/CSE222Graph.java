package homework8;
 
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.Map;


/**
 * Represents a map with obstacles and start/end points.
 */
public class CSE222Graph {
    // String reperansation of coordinates of vertexes as y,x
    private Map<String, ArrayList<String>> vertexes;
    private String start;
    private String end;
    private int numV;

    /**
     * Constructs a CSE222Graph object by creating nodes from the CSE222map.
     *
     * @param map the CSE222Map to the variable containing the map data
     */
    public CSE222Graph(CSE222Map map) {
        int mapy = map.getMap().size();
        vertexes = new LinkedHashMap<String,ArrayList<String>>();
        start = map.getStartX() + "," + map.getStartY();
        end = map.getEndX() + "," + map.getEndY();
        
        for (int y = 0; y < mapy; y++) {
            int mapx = map.getMap().get(y).size();
            for (int x = 0; x < mapx; x++) { 
                if(map.getMap().get(y).get(x) == 0)
                {    
                    ++numV;
                    vertexes.put(y + "," + x, new ArrayList<>());
                    for (int i = -1; i < 2; ++i) {
                        for (int j = -1; j < 2; j++) {
                            if(!(y+i < 0 || y+i >= mapy) && !(x+j < 0 || x+j >= mapx) && map.getMap().get(y+i).get(x+j) == 0 && !(i == 0 && j == 0)) {
                                vertexes.get(y + "," + x).add((y + i) + "," + (x + j));
                            }
                        }
                    }
                }
            }
        }    
    
    }

     /**
     * Gets the graph.
     *
     * @return the node represention of the graph
     */
    public Map<String, ArrayList<String>> getMap() {
        return vertexes;
    }

    /**
     * Gets the neighbor vertexes of the given vertex.
     * 
     * @param vertex the vertex to get the neighbors of
     * @return the neighbor vertexes of the given vertex
     */
    public ArrayList<String> getNeigbors(String vertex) {
        return vertexes.get(vertex);
    }

    /**
     * Gets the number of the verticexes.
     *
     * @return the number of the verticexes
     */
    public int getNumV() {
        return numV;
    }

    /**
     * Gets the coordinates of the starting point.
     *
     * @return the coordinates of the starting point
     */
    public String getStart() {
        return start;
    }

    /**
     * Gets the coordinates of the ending point.
     *
     * @return the coordinates of the ending point
     */
    public String getEnd() {
        return end;
    }


}
