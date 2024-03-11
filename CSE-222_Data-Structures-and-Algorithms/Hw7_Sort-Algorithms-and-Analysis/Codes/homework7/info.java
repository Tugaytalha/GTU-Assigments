package homework7;

import java.util.ArrayList;


/**
 * The info class represents a container for storing information about words that have a specific letter and its count.
 */
public class info implements Cloneable {

    private int count;
    private ArrayList<String> words;

    /**
     * Constructs an info object with the given word.
     *
     * @param word first word to stored
     */
    public info(String word) {
        words = new ArrayList<>();
        words.add(word);
        count = 1;
    }

    /**
     * Constructs an empty info object with zero count and an empty list of words.
     */
    public info() {
        words = new ArrayList<>();
        count = 0;
    }

    /**
     * Adds a new word to the list of words and increments the count.
     *
     * @param word the new word to added
     */
    public void push(String word) {
        words.add(word);
        ++count;
    }
   
    /**
     * Creates and returns a new info object that is a copy of this one.
     *
     * @return a clone of this info object
     * @throws CloneNotSupportedException if the info class is not cloneable
     */
    @Override
    public info clone() throws CloneNotSupportedException {
        // Call the superclass's clone method to create a shallow copy
        info clone = (info) super.clone();

        clone.words = new ArrayList<>(words);

        return clone;
    }

    /**
     * Returns the count of letter occurs.
     *
     * @return The count of  letter occurs.
     */
    public int getCount() {
        return count;
    }

    /**
     * Returns a string representation of this info object, including the count and the list of words.
     *
     * @return a string representation of this info object, including the count and the list of words.
     */
    @Override
    public String toString() {
        return "Count: " + count + " - Words: " + words.toString();
    }


}

