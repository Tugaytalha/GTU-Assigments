package homework1;

/**
 * This class represents a comment on a post.
 */
public class Comment extends Interaction {

    private static int commentCount = 0;
    private String content;

    /**
     * Construct that takes content, account and post.
     * @param accId The account ID.
     * @param postId The ID of the post that the comment was made on.
     * @param conten The content of the comment.
     */
    public Comment(int accId, int postId, String conten) {
        super(accId, postId);
        commentCount++;
        interactionId = commentCount;
        this.content = conten;
    }

    /**
     * Returns the content.
     * @return The content of this comment.
     */
    public String getContent() {
        return content;
    }
}