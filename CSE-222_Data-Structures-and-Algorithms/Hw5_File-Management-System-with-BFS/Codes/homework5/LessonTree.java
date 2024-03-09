package homework5;

import java.io.File;
import java.io.FileNotFoundException;
import java.util.Scanner;
import java.util.ArrayList;
//import java.util.Arrays;
import java.util.Queue;
import java.util.LinkedList;
//import java.util.Map; 
//import java.util.HashMap;
import java.util.Stack;
import javax.swing.JFrame;
import javax.swing.JTree;
import javax.swing.tree.DefaultMutableTreeNode;

/**
 * A class that reads data from a .txt file and represents it as a tree structure.
 * Each row of the .txt file represents a data point, and is split into categories by the ";" character.
 */
public class LessonTree {
    private JTree tree;

    /**
     * Constructs a LessonTree object and initializes the root.
     */
    public LessonTree() {
        DefaultMutableTreeNode root = new DefaultMutableTreeNode("Root"); 
        tree = new JTree(root);
    }

    /**
     * Reads the data from the given file and builds a tree structure.
     * The tree is represented by a Jtree object with a root node labeled
     * as "Root", and each data point in the file is added to the tree as a node.
     *
     * @param fileName the name of the file to read
     * @throws FileNotFoundException if the file is not found
     */
    public void buildTree(String fileName) throws FileNotFoundException {
        DefaultMutableTreeNode root = (DefaultMutableTreeNode) tree.getModel().getRoot();
        File file = new File(fileName);
        Scanner scanner = new Scanner(file);
        ArrayList<String[]> arr = new ArrayList<String[]>();

        // Read file and add to 2d string array
        while (scanner.hasNextLine()) 
            arr.add(scanner.nextLine().split(";"));
        
        
        for (int i = 0; i < arr.size(); i++) {
            addToTree(root, arr.get(i));
        }
        scanner.close();
    }

    /**
     * Adds a node to a Tree using root node as a DefaultMutableTreeNode.
     *
     * @param root the root node of the tree
     * @param node_arr an array of strings representing the path to the node being added
     * @return {@code true} if the node was added normally, {@code false} if the node already exists and overwrited 
     */
    private boolean addToTree(DefaultMutableTreeNode root, String[] node_arr) {
        DefaultMutableTreeNode parent;

            if (childIndex(root, node_arr[0]) == -1) {
                parent = new DefaultMutableTreeNode(node_arr[0]);
                root.add(parent);
            }
            else{
                parent = (DefaultMutableTreeNode) root.getChildAt(childIndex(root, node_arr[0]));
            }
            for (int j = 1; j < node_arr.length; j++) {
                DefaultMutableTreeNode child;
                if (childIndex(parent, node_arr[j]) == -1) {
                    child = new DefaultMutableTreeNode(node_arr[j]);
                    parent.add(child);
                }
                else if(j == node_arr.length - 1){
                    child = new DefaultMutableTreeNode(node_arr[j]);
                    parent.add(child);
                    return false;
                }
                else {
                    child = (DefaultMutableTreeNode) parent.getChildAt(childIndex(parent, node_arr[j]));
                }
                parent = child;
            }
            return true;
    }


    /**
     * Displays the tree in a JFrame.
     * The JFrame contains the JTree object, which is readed from the file.
     */
    public void displayTree() {
        JFrame frame = new JFrame("Tree");
        frame.add(tree);
        frame.pack();
        frame.setVisible(true);
        frame.setSize(300,430);  
        for (int i = 0; i < tree.getRowCount(); i++) {
            tree.expandRow(i);
        }
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        
    }
    
    /**
     * Returns the index of the specified child node in the parent node.
     * 
     * @param parent the parent node to search for the child node
     * @param child the child content to find the index for
     * @return the index of the child node in the parent node, or -1 if not found
     */
    private int childIndex(DefaultMutableTreeNode parent,Object child) {
        for(int i = 0; i < parent.getChildCount(); i++) {
            if (((DefaultMutableTreeNode) parent.getChildAt(i)).getUserObject().equals(child)){
                return i;
            }
        }
        return -1;
    }

    /**
     * Apply BFS algorithm to find an input in the tree.
     *
     * @param element element content to be founded
     */
    public int bfsSearch(Object element) {
        System.out.println("Using BFS to find '" + element + "' in the tree...");

        // Create an empty queue and add the root node
        Queue<DefaultMutableTreeNode> queue = new LinkedList<>();
        queue.add((DefaultMutableTreeNode) tree.getModel().getRoot());
        int step = 0;

        // Process each node in the queue
        while (!queue.isEmpty()) {
            DefaultMutableTreeNode node = queue.poll();

            System.out.print("Step " + ++step + " -> " + node.getUserObject());
            if(node.getUserObject().equals(element)) {
                System.out.println("(Found!)");
                System.out.println();
                return step;
            }
            System.out.println();
            
            // Add the children of the current node to the queue
            for (int i = 0; i < node.getChildCount(); i++) {
                DefaultMutableTreeNode child = (DefaultMutableTreeNode) node.getChildAt(i);
                queue.add(child);
            }
        }
        System.out.println("Not Found in this Tree!");
        System.out.println();
        return -1;
    }
    
    /**
     * Apply DFS algorithm to find an input in the tree.
     *
     * @param element element content to be founded
     */
    public int dfsSearch(Object element) {
        System.out.println("Using DFS to find '" + element + "' in the tree...");

        // Create an empty stack and add the root node
        Stack<DefaultMutableTreeNode> stack = new Stack<>();
        stack.add((DefaultMutableTreeNode) tree.getModel().getRoot());
        int step = 0;

        // Process each node in the stack
        while (!stack.isEmpty()) {
            DefaultMutableTreeNode node = stack.pop();

            System.out.print("Step " + ++step + " -> " + node.getUserObject());
            if(node.getUserObject().equals(element)) {
                System.out.println("(Found!)");
                System.out.println();
                return step;
            }
            System.out.println();
            
            // Add the children of the current node to the stack
            for (int i = 0; i < node.getChildCount(); i++) {
                DefaultMutableTreeNode child = (DefaultMutableTreeNode) node.getChildAt(i);
                stack.add(child);
            }
        }
        System.out.println("Not Found in this Tree!");
        System.out.println();
        return -1;
    }

 
    /**
     * Apply post order traversal algorithm to find an input in the tree.
     *
     * @param element element content to be founded
     */
    public int potSearch(Object element) {
        System.out.println("Using post order traversal to find '" + element + "' in the tree...");

        // Create an empty queue and add the root node
        Queue<DefaultMutableTreeNode> queue = new LinkedList<>();
        DefaultMutableTreeNode node = (DefaultMutableTreeNode) tree.getModel().getRoot();
        while(!(node.getChildCount() <=0))
            node = (DefaultMutableTreeNode) node.getChildAt(0);
        queue.add(node);
        int step = 0;

        // Process each node in the queue
        while (!queue.isEmpty()) {
            DefaultMutableTreeNode cursor = queue.poll();

            System.out.print("Step " + ++step + " -> " + cursor.getUserObject());
            if(cursor.getUserObject().equals(element)) {
                System.out.println("(Found!)");
                System.out.println();
                return step;
            }
            System.out.println();
            
            // Add the children of the current node to the queue
            while(node.getNextSibling() != null) {
                node = (DefaultMutableTreeNode) node.getNextSibling();
                while(!(node.getChildCount() <=0))
                    node = (DefaultMutableTreeNode) node.getChildAt(0);
                queue.add(node);
            }

            if(node.getParent() == null) 
                continue;
            
            node = (DefaultMutableTreeNode) node.getParent();
            queue.add(node);
            
            if(node.getNextSibling() == null)
                continue;
            node = (DefaultMutableTreeNode) node.getNextSibling();
            while(!(node.getChildCount() <=0))
                node = (DefaultMutableTreeNode) node.getChildAt(0);
            queue.add(node);
        }
        System.out.println("Not Found in this Tree!");
        System.out.println();
        return -1;
    }
    
    /**
     * Move a problem/lecture/course (any node which is not root or not the child 
     * of root) from one year to another. If the problem/lecture/course exists in 
     * the destination year, overwrite it and inform the user about it.
     * 
     * @param source string array represention of source node
     * @param destination string repersation of destination year
     * @return {@code true} if move is succesfull {@code false} otherwise
     */
    public boolean moveNode(String[] source, String destination) {
        DefaultMutableTreeNode root = (DefaultMutableTreeNode) tree.getModel().getRoot();
        DefaultMutableTreeNode source_node = root;
        DefaultMutableTreeNode dest_node = root;
        DefaultMutableTreeNode delete_node;
        String source_year = source[0];
        for(int i = 0; i < source.length; ++i){
            int index = childIndex(source_node, source[i]);
            if (index != -1)
                source_node = (DefaultMutableTreeNode) source_node.getChildAt(index);
            else {
                System.out.printf("Cannot move %s", source[0]);
                for(int f = 1; f < source.length; ++f)
                    System.out.printf("->%s", source[f]);
                System.out.println(" beacuse it doesn't exist in tree");
                return false;
            }
        }
        delete_node = source_node;
        while(delete_node.getParent().getChildCount() == 1)
            delete_node = (DefaultMutableTreeNode) delete_node.getParent();
        
        if(delete_node.getParent() == null) {
            delete_node = (DefaultMutableTreeNode) delete_node.getChildAt(0);
        }
        ((DefaultMutableTreeNode) delete_node.getParent()).remove(delete_node);
        source[0] = destination;

        System.out.printf("Moved %s", source_year);
        for(int f = 1; f < source.length; ++f)
            System.out.printf("->%s", source[f]);
        System.out.println(" to " + destination);

        if(!addToTree(root, source)) {
            for(int i = 0; i < source.length - 1; ++i){
                int index = childIndex(dest_node, source[i]);
                dest_node = (DefaultMutableTreeNode) dest_node.getChildAt(index);
            }
            int index = childIndex(dest_node, source[source.length - 1]);
            dest_node.remove(index);
            dest_node.add(source_node);
            for(int f = 1; f < source.length; ++f)
                System.out.printf("->%s", source[f]);
            System.out.println(" has been overwritten.");
        }

        this.displayTree();

        return true;
    }


}