package homework6;

/**
 * This is the main class for running the program to sort maps. 
 * It takes three example input strings, builds a map for each, 
 * sorts each map using mergeSort, and displays the sorted maps.
 */
public class Main {
   
    /**
     * The main method of the program. It creates three example input strings, 
     * builds a map for each, sorts each map using mergeSort, and displays the sorted maps. 
     * 
     * @param args command-line arguments, not used.
     */
    public static void main(String[] args) {
        String[] exStrs = new String[3];
        exStrs[0] = "Buzzing bees buzz.";
        exStrs[1] = "'Hush, hush!' whispered the rushing wind.";
        exStrs[2] = "ThiS is An Examp!e InpUt StRinG.";

        myMap[] maps = new myMap[3];
        mergeSort[] sorts = new mergeSort[3];

        for (int i = 0; i < 3; ++i) {
            maps[i] = new myMap(exStrs[i]);
            maps[i].preProcess();
            maps[i].buildMap();
            sorts[i] = new mergeSort(maps[i]);
            sorts[i].sortMap();
        }
    }
}
