package homework1;


/**
 * This abstract class represents an interaction such as a like or a comment.
 */
public abstract class Interaction {
    
    protected int interactionId;
    protected int accountId;
    protected int postId;
    
    /**
     * Creates a new interaction object with the given account ID and post ID.
     * @param accountId The ID of the account that made the interaction.
     * @param postId The post ID that interacted .
     */
    public Interaction(int accountId, int postId) {
        this.accountId = accountId;
        this.postId = postId;
    }
    
    /**
     * Gets interaction ID.
     * @return The interaction ID.
     */
    public int getId() {
        return interactionId;
    }
    
    /**
     * Gets account ID that make inte4raction.
     * @return The account ID.
     */
    public Integer getAccountId() {
        return accountId;
    }
    
    /**
     * Gets post ID that interacted.
     * @return The post ID.
     */
    public Integer getPostId() {
        return postId;
    }
}