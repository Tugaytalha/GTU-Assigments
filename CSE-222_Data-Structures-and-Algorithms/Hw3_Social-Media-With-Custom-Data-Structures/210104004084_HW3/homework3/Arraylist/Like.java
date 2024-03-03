package homework3.ArrayList;

/**
 * This class represents a like interaction.
 */
public class Like extends Interaction {

    private static int likeCount = 0;

    /**
     * Constructor that takes accountId and postId.
     * @param accountId The ID of user that likes the post.
     * @param postId The ID of the post being liked.
     */
    public Like(int accountId, int postId) {
        super(accountId, postId);
        likeCount++;
        interactionId = likeCount;
    }

    /**
     * Gets the total number of likes.
     * @return The total number of likes.
     */
    public int getId() {
        return interactionId;
    }
}