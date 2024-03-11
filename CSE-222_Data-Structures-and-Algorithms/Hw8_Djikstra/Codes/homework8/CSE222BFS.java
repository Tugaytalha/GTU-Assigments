package homework8;
 
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.LinkedList;
import java.util.Map;
import java.util.Queue;


/**
 * Represents a map with obstacles and start/end points.
 */
public class CSE222BFS {
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
    public CSE222BFS(CSE222Graph graph) {
        this.graph = graph;
        this.start = graph.getStart();
        this.end = graph.getEnd();
    }

    /**
     * Perform a breadth first search of a graph.
     * 
     * @return The array of parents
     */
    public ArrayList<String> findPath() {
        Queue<String> theQueue = new LinkedList<>();
        // Declare array parent and initialize its elements to –1.
        Map<String, String> parent = new LinkedHashMap<>();
        for (var i: graph.getMap().keySet()) {
            parent.put(i, null);
        }
        
        
        // Declare array identified and initialize its elements to false.
        Map<String, Boolean> identified = new LinkedHashMap<>();
        for (var i: graph.getMap().keySet()) {
            identified.put(i, false);
        }

        /* Mark the start vertex as identified and insert it into the queue */
        identified.put(start, true);
        theQueue.offer(start);
        
        /* Perform breadth first search until done */
        System.out.println(theQueue.isEmpty());
        while (!theQueue.isEmpty()) {
            /* Take a vertex, current, out of the queue. (Begin visiting current). */
            String current = theQueue.remove();
            
            
            /* Examine each vertex, neighbor, adjacent to current. */
            ArrayList<String> neigbors = graph.getNeigbors(current);
            
            System.out.println(theQueue.isEmpty());
            if (neigbors != null){
                for (var neighbor: neigbors) {
                    
                    // If neighbor has not been identified
                    if (!identified.get(neighbor)) {
                        // Mark it identified.
                        identified.put(neighbor, true);
                        
                        // Place it into the queue.
                        theQueue.offer(neighbor);
                        
                        /* Insert the edge (current, neighbor) into the tree. */
                        parent.put(neighbor, current);
                    }
                }
            }
            // Finished visiting current.
        }
        
        return shortestPath(parent);
    }

    public ArrayList<String> shortestPath(Map<String, String> parent) { 
         
        // Initialize the shortest path array with end vertex 
        ArrayList<String> shortestPath = new ArrayList<>(); 
        shortestPath.add(end); 
         
        // Traverse the parent map from end to start to obtain the shortest path 
        String current = end; 
        while (!current.equals(start)) { 
            String parentVertex = parent.get(current); 
            System.out.println(parentVertex);
            shortestPath.add(0, parentVertex); 
            current = parentVertex; 
        } 
         
        // Update the length of the shortest path 
        length = shortestPath.size() - 1; 
         
        return shortestPath; 
    }

}