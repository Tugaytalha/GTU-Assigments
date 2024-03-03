package homework3.LinkedList;

/**
 * This class represents a social media message.
 */
public class Message {

    private int messageId;
    private int senderId;
    private int receiverId;
    private String content;
    private static int messagecount = 0;
    
    /**
     * Constructor that takes sender ID, receiver ID, and content.
     * @param sender The ID of the user who sent the message.
     * @param receiver The ID of the user who will receive the message.
     * @param conten The content of the message.
     */
    public Message(Account sender, Account receiver, String conten) {
        messagecount++;
        messageId = messagecount;
        this.senderId = sender.getID();
        this.receiverId = receiver.getID();
        this.content = conten;
    }

    /**
     * Returns the content of the message.
     * @return The content of the message.
     */
    public String getContent() {
        return content;
    }

    /**
     * Gets the sender ID
     * @return The ID of the sender.
     */
    public int getSenderId() {
        return senderId;
    }

    /**
     * Gets the receiver ID
     * @return The ID of the receiver.
     */
    public int getReceiverId() {
        return receiverId;
    }

    /**
     * Gets the message ID
     * @return The ID of the message.
     */
    public int getId() {
        return messageId;
    }
}