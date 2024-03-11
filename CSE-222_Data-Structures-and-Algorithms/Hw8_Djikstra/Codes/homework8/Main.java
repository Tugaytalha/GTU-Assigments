package homework8;

import java.util.ArrayList;

/**
 * This is the main class for running the program to sort maps. 
 * It takes two example input strings, builds a map for each, 
 * sorts each map using mergeSort, and displays the sorted maps.
 */
public class Main {
   
    /**
     * The main method of the program. It creates two example input strings, 
     * builds a map for each, sorts each map using 5 sort algrothms, and displays the sorted maps. 
     * 
     * @param args command-line arguments, not used.
     */
    public static void main(String[] args) {
        
        String InputFile = "./homework8/TextFiles/" + "Map01.txt"; 
        String OutputFileBFS = "./homework8/PngFiles/" + "Map01B.png";
        String OutputFileDijkstra = "./homework8/PngFiles/" + "Map01D.png";
        String TxtBFS = "./homework8/paths/" + "Map01B.txt";
        String TxtDijkstra = "./homework8/paths/" + "Map01D.txt";
        int X_LENGTH = 500;
        int Y_LENGTH = 500; 
        CSE222Map Map = new CSE222Map(InputFile);
        CSE222Graph Graph = new CSE222Graph (Map);
        CSE222Dijkstra Dijkstra = new CSE222Dijkstra (Graph); 
        System.out.println("scxzcz1");
//        ArrayList<String> DijkstraPath = Dijkstra.findPath();
        System.out.println("scxzcz2");
        CSE222BFS BFS= new CSE222BFS (Graph); 
        System.out.println("scxzcz3");
        ArrayList<String> BFSPath = BFS.findPath();
        System.out.println("scxzcz4");
        Map.convertPNG(OutputFileBFS);
        System.out.println("scxzcz4");
//        Map.convertPNG(OutputFileDijkstra);
        System.out.println("scxzcz4");
//        Map.drawLine(DijkstraPath, OutputFileDijkstra);
        Map.drawLine(BFSPath, OutputFileBFS);
        Map.writePath(BFSPath, TxtBFS);
//        Map.writePath(DijkstraPath, TxtDijkstra);
//        System.out.println("Dijkstra Path: "+ Dijkstra.length); 
        System.out.println("BFS Path: "+ BFS.length);

    }

}
