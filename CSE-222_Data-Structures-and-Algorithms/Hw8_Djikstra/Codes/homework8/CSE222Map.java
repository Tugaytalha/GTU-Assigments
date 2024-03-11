package homework8;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Scanner;
import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import javax.imageio.ImageIO;    
import java.awt.*;

/**
 * Represents a map with obstacles and start/end points.
 */
public class CSE222Map {
    private ArrayList<ArrayList<Integer>> map;
    private int startX;
    private int startY;
    private int endX;
    private int endY;

    /**
     * Constructs a CSE222Map object by reading the map data from a file.
     *
     * @param filePath the path to the text file containing the map data
     */
    public CSE222Map(String filePath) {
        try {
            map = new ArrayList<ArrayList<Integer>>();
            File file = new File(filePath);
            Scanner scanner = new Scanner(file);

            // Read start point coordinates
            String[] startCoords = scanner.nextLine().split(",");
            startY = Integer.parseInt(startCoords[0].trim());
            startX = Integer.parseInt(startCoords[1].trim());

            // Read end point coordinates
            String[] endCoords = scanner.nextLine().split(",");
            endY = Integer.parseInt(endCoords[0].trim());
            endX = Integer.parseInt(endCoords[1].trim());

            // Read map values
            int row = 0;
            while (scanner.hasNextLine()) {
                map.add(new ArrayList<>());
                String[] lineValues = scanner.nextLine().split(",");
                for (int col = 0; col < lineValues.length; col++) {
                    int value = Integer.parseInt(lineValues[col].trim().replace("-1", "1"));
                    map.get(row).add(value);
                }
                row++;
            }

            scanner.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

    }

     /**
     * Gets the map matrix.
     *
     * @return the 2D matrix representing the map
     */
    public ArrayList<ArrayList<Integer>> getMap() {
        return map;
    }

    /**
     * Gets the x-coordinate of the starting point.
     *
     * @return the x-coordinate of the starting point
     */
    public int getStartX() {
        return startX;
    }

    /**
     * Gets the y-coordinate of the starting point.
     *
     * @return the y-coordinate of the starting point
     */
    public int getStartY() {
        return startY;
    }

    /**
     * Gets the x-coordinate of the ending point.
     *
     * @return the x-coordinate of the ending point
     */
    public int getEndX() {
        return endX;
    }

    /**
     * Gets the y-coordinate of the ending point.
     *
     * @return the y-coordinate of the ending point
     */
    public int getEndY() {
        return endY;
    }

    public void convertPNG(String filePath) {
        try {
            int mapsize = map.size();

            BufferedImage bufferedimage = new BufferedImage(mapsize, mapsize, BufferedImage.TYPE_INT_RGB);
            Graphics2D g2d = bufferedimage.createGraphics();

            for (int i = 0; i < mapsize; i++) {
                for (int j = 0; j < mapsize; j++) {
                    if (map.get(i).get(j) == 1) {
                        g2d.fillRect(j, i, 1, 1);
                    }
                }
            }

            // save this image
            ImageIO.write(bufferedimage, "PNG", new File(filePath));
            
        } catch (Exception e) {
            e.printStackTrace();
        }
        
    }

    private void setBackgroundColor(Graphics2D graphics, int mapSize) {
        graphics.setBackground(Color.WHITE);
        graphics.clearRect(0, 0, mapSize, mapSize);
    }

    public void drawLine(ArrayList<String> path, String filePath) {
        try {
            int mapSize = map.size();
            BufferedImage bufferedimage = new BufferedImage(mapSize, mapSize, BufferedImage.TYPE_INT_RGB);
            Graphics2D graphics = bufferedimage.createGraphics();

            setBackgroundColor(graphics, mapSize);

            graphics.setColor(Color.BLACK);
            for (int i = 0; i < path.size(); i++) {
                String[] coords = path.get(i).split(",");
                int x = Integer.parseInt(coords[0].trim());
                int y = Integer.parseInt(coords[1].trim());
                graphics.fillRect(y, x, 1, 1);
            }

            // save this image
            ImageIO.write(bufferedimage, "PNG", new File(filePath));
            
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void writePath(ArrayList<String> path, String outputPath) {
        try {
            FileWriter writer = new FileWriter(outputPath);
            for (String vertex : path) {
                writer.write(vertex + "\n");
            }
            writer.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}
