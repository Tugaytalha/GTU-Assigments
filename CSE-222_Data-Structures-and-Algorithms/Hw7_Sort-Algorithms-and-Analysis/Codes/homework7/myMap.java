package homework7;


import java.util.ArrayList;
import java.util.LinkedHashMap;

/**
 * A custom map structure that preprocesses a given string and builds a map based on it.
 */
public class myMap  implements Cloneable{

    private LinkedHashMap <String, info> map; 
    private int mapSize;
    private String str;

    /**
     * Constructs a myMap object with a default input string "ThiS is An Examp!e InpUt StRinG." and an empty map.
     */
    public myMap() {
        map = new LinkedHashMap<>();
        mapSize = 0;
        str = "ThiS is An Examp!e InpUt StRinG.";
    }

    /**
     * Constructs a myMap object with the specified input string and an empty map. 
     * if specified input string is empty or null that will be defaulted to "ThiS is An Examp!e InpUt StRinG."
     * 
     * @param str The input string to be processed and used to build the map.
     */
    public myMap(String str) {
        if(str == null || str.isEmpty()) {
            str = "ThiS is An Examp!e InpUt StRinG.";
            System.out.println("String cannot be empty, defaulted string: " + str);
        }
        map = new LinkedHashMap<>();
        mapSize = 0;
        this.str = str;
    }
   
    /**
     * Takes in a String and preprocesses it by removing all non-alphabetic 
     * characters and converting all characters to lowercase.
     * 
     * @return preProcessed string
     */
    public String preProcess() {
        
        System.out.println("Orginal String:             " + str);
        str = str.replaceAll("[^a-zA-Z ]", "").toLowerCase();
        System.out.println("Preprocessed String:        " + str);
        System.out.println("\n\n");

        
        return str;
    }

    /**
     * Build a map with letters of input string, each letter have a info object
     * that keeps letter's count and words that contains that letter.  
     * 
     * @return The string that was used to build the map.
     */
    public String buildMap() {
        // Split the input string into an array of words.
        String[] words_array = str.split(" ");
        
        for(var i : words_array){
            // If the character exists in the map, add the word to the list associated with the character.
            for(Character j : i.toCharArray()){
                if(map.keySet().contains(j.toString())) {
                    map.get(j.toString()).push(i);;
                }
                else {
                    info firstInfo = new info(i);
                    this.put(j.toString(), firstInfo);
                }
            }
        }
        
        return str;
    }

    /**
     * Creates and returns a deep copy of this myMap object.
     *
     * @return a new myMap object that is a copy of this one
     */
    @Override
    public myMap clone() {
        try {
            // Call the superclass's clone method to create a shallow copy
            myMap clonedMap = (myMap) super.clone();
            clonedMap.map = new LinkedHashMap<>();
            clonedMap.mapSize = 0;
            for (String key : this.map.keySet()) {
                info originalValue = this.map.get(key);
                clonedMap.put(key, originalValue.clone());
            }
            return clonedMap;
        } catch (CloneNotSupportedException e) {
            throw new AssertionError();
        }
    }

    /**
     * Returns the map object of this class as a LinkedHashMap.
     * 
     * @return The map object of this class as a LinkedHashMap.
     */
    public LinkedHashMap<String, info> getMap() {
        return map;
    }

    /**
     * Returns the size of the map.
     * @return The size of the map.
     */
    public int size() {
        return mapSize;
    }

    /** 
     * Returns the the string of class .
     * @return The string of class.
     */
    public String getStr() {
        return str;
    }

    /**
     * Returns an ArrayList of String keys in the map.
     * @return The ArrayList of String keys in the map
     */
    public ArrayList<String> keyArrayList() {
        ArrayList<String> returnArr = new ArrayList<String>();
        returnArr.addAll(0, map.keySet());
        return returnArr;
    }

    /**
     * Returns the info object of the gicen letter.
     * 
     * @param key The String key for which to retrieve the associated info object.
     * @return The info object of the given letter
     */
    public info get(String key) {
        return map.get(key);
    }

    /**
     * Returns the count of the spesific letter
     * 
     * @param key The letter to return count
     * @return The count of the given letter
     */
    public int getCount(String key) {
        return map.get(key).getCount();
    }

    /**
     * Adds a new letter and it's info class to map .
     * 
     * @param key the letter to be added to map
     * @param value The info object to be added
     * @return added info object
     */
    public info put(String key, info value) {
        ++mapSize;
        return map.put(key, value);
    }

    /**
     * Print the map in Letter:  - Count:  - Words: format format 
     * using info class tostring 
     * 
     */
    public void show() {
        String[] keyArr = map.keySet().toArray(new String[mapSize]);

        for(var i : keyArr)
            System.out.println("Letter: " + i + " - " + get(i).toString());
        System.out.println("\n");
    }

    /**
     * Return string represention of the map in 
     * Letter:  - Count:  - Words: format format using info class tostring 
     */
    @Override
    public String toString() {
        String[] keyArr = map.keySet().toArray(new String[mapSize]);
        String returnStr = new String();

        for(var i : keyArr)
            returnStr = returnStr + "Letter: " + i + " - " + get(i).toString() + "\n";
        return returnStr;
    }

    /**
     * Another representation for show
     */
    public void print() {
        show();
    }



}

