package homework8;
 
import java.util.ArrayList;
import java.util.HashSet;
import java.util.LinkedHashMap;
import java.util.Map;


/**
 * Represents a map with obstacles and start/end points.
 */
public class CSE222Dijkstra {
    // String reperansation of coordinates of vertexes as y,x
    private CSE222Graph graph;
    private String start;
    private String end; 
    // The length of the shortest path.
    public int length;  

    /**
     * Constructs a CSE222Djkstra object with the given CSE222Graph.
     *
     * @param graph the CSE222graph 
     */
    public CSE222Dijkstra(CSE222Graph graph) {
        this.graph = graph;
        this.start = graph.getStart();
        this.end = graph.getEnd();
    }

    /** 
     * Dijkstra's Shortest Path algorithm.
     * 
     * @return Output array to contain the predecessors in the shortest path     
     */
    public ArrayList<String> findPath() {
        int numV = graph.getNumV();
        HashSet<String> vMinusS = new HashSet<>(numV);
        Map<String, String> pred = new LinkedHashMap<>();
        Map<String, Integer> dist = new LinkedHashMap<>();
    
        // Initialize V–S.
        for (var i: graph.getMap().keySet()) {
            if (!i.equals(start)) {
                vMinusS.add(i);
            }
        }
        dist.put(start, 0);
    
        // Initialize pred and dist.
        for (var i : vMinusS) {
            pred.put(i, start);
        }
    
        // Main loop
        while (vMinusS.size() != 0) {
            // Find the value u in V–S with the smallest dist[u].
            double minDist = Double.POSITIVE_INFINITY;
            String u = null;
    
            for (var v : vMinusS) {
                if(dist.get(v) != null){
                    if (dist.get(v) < minDist) {
                        minDist = dist.get(v);
                        u = v;
                    }
                }
            }
    
            // Remove u from vMinusS.
            vMinusS.remove(u);
    
            // Update the distances.
            for (var v : vMinusS) {
                if(graph.getNeigbors(u) != null){
                    if (graph.getNeigbors(u).contains(v)) {
                        if (dist.get(u) + 1 < dist.get(v)) {
                            dist.put(v, dist.get(u) + 1);
                            pred.put(v, u);
                        }
                    }
                }
            }
        }
        return shortestPath(pred);
    }

    public ArrayList<String> shortestPath(Map<String, String> parent) { 
         
        // Initialize the shortest path array with end vertex 
        ArrayList<String> shortestPath = new ArrayList<>(); 
        shortestPath.add(end); 
         
        // Traverse the parent map from end to start to obtain the shortest path 
        String current = end; 
        while (!current.equals(start)) { 
            String parentVertex = parent.get(current); 
            shortestPath.add(0, parentVertex); 
            current = parentVertex; 
        } 
         
        // Update the length of the shortest path 
        length = shortestPath.size() - 1; 
         
        return shortestPath; 
    }


}