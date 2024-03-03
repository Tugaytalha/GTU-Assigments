package homework3.LDLinkedList;

import java.util.AbstractList;
import java.util.List;


/**
 * A linked list that uses lazy deletion.
 *
 * @param <E> the type of elements in this list
 */
public class LDLinkedList<E> extends AbstractList<E> implements List<E> {

    /** First node referance */
    private Node<E> head;
    /**last node referance */
    private Node<E> tail;
    /** Non-deleted size */
    private int ndsize;
    /** size */
    private int size;
    /** Index of lazy deleted element */
    private int lazyIndex = -1;

    /**
     * A node of linked list.
     */
    private static class Node<E> {
        /** The element stored in node. */
        E data;
        /** next node reference  */
        Node<E> next;
        /** A flag indicating whether this node is lazily deleted. */
        boolean deleted;

        /**
         * Constructs a new node with the specified data.
         *
         * @param data the element to be stored in this node
         */
        Node(E data) {
            this.data = data;
            this.deleted = false;
        }
    }

    /**
     * Constructs an empty linked list.
     */
    public LDLinkedList() {
        head = null;
        tail = null;
        size = 0;
    }

    /**
     * Appends the element to the end of this list.
     *
     * @param data the element to be added
     * @return True
     */
    @Override
    public boolean add(E data) {
        Node<E> newNode = new Node<>(data);
        if (head == null) {
            head = newNode;
            tail = newNode;
        } else {
            tail.next = newNode;
            tail = newNode;
        }
        size++;
        ndsize++;
        return true;
    }

    /**
     * Returns the element at the specified position.
     *
     * @param index the index of the element to be returned
     * @return the element at the specified position
     * @throws IndexOutOfBoundsException if the index is out of range
     *         
     */
    @Override
    public E get(int index) {
        Node<E> current = head;
        int currentIndex = 0;
        while (current != null) {
            if (!current.deleted && currentIndex == index) {
                return current.data;
            }
            currentIndex++;
            current = current.next;
        }
        //System.out.println("IndexOutOfBoundsException");
        return null;
    }

    /**
     * Replaces the element at the specified position in this list with the
     * specified element.
     *
     * @param index the index of the element to replace
     * @param element the new element 
     * @return the element previously at the specified position
     * @throws IndexOutOfBoundsException if the index is out of range
     */
    @Override
    public E set(int index, E element) {
        if (index < 0 || index >= size()) {
            throw new IndexOutOfBoundsException("Index: " + index + ", Size: " + size());
        }
        Node<E> current = head;
        int i = 0;
        while (current != null && i < index) {
            if (!current.deleted) {
                i++;
            }
            current = current.next;
        }
        if (current != null) {
            E oldElement = current.data;
            current.data = element;
            return oldElement;
        }
        return null;
    }
    
    /**
     * Lazily deletes the element at the specified position removes any adjacent lazily deleted nodes.
     *
     * @param index the index of the element to be removed
     * @return the element that was removed from the list
     * @throws IndexOutOfBoundsException if the index is out of range
     */
    @Override
    public E remove(int index) {
        Node<E> current = head;
        Node<E> previous;
        int currentIndex = 0;
        while (current != null) {
            if (currentIndex == index) {
                current.deleted = true;
                ndsize--;
                if(lazyIndex == -1) 
                    lazyIndex = currentIndex;
                else {
                    if(lazyIndex > currentIndex) {
                        removeNode(lazyIndex); 
                        removeNode(currentIndex);
                    }
                    else if (currentIndex == lazyIndex) {
                        removeNode(lazyIndex); 
                    }
                    else {
                        removeNode(currentIndex); 
                        removeNode(lazyIndex); 
                    }
                    lazyIndex = -1;
                }
                return current.data;
            }
            currentIndex++;
            previous = current;
            current = current.next;
        }
        throw new IndexOutOfBoundsException("Index: " + index + ", Size: " + size());
    }

    /**
     * Removes the specified node from the list.
     *
     * @param node the node to be removed
     */
    private void removeNode(int index) {
        Node<E> current = head;
        Node<E> previous = null;
        int currentIndex = 0;

        while (current != null) {
            if (currentIndex == index) {
                if (previous == null) {
                    head = current.next;
                } else {
                    previous.next = current.next;
                }
                if (current.next == null) {
                    tail = previous;
                }
                size--;
                break;
            }
            currentIndex++;
            previous = current;
            current = current.next;
        }
    }

    /**
     * Returns the number of elements.
     *
     * @return the number of elements
     */
    @Override
    public int size() {
        return size;
    }

    /**
     * Returns the number of non-deleted elements.
     *
     * @return the number of non-deleted elements
     */
    public int ndsize() {
        return ndsize;
    }

    public int find(E element) {
        int index = 0;
        Node<E> currentNode = head;
    
        while (currentNode != null) {
            if (currentNode.data.equals(element) && !currentNode.deleted) {
                return index;
            }
            currentNode = currentNode.next;
            index++;
        }
    
        // Element not found in the list
        return -1;
    }

}