package homework5;

import java.io.FileNotFoundException;
import java.util.Scanner;

public class Main {
   
    public static void main(String[] args) throws FileNotFoundException {
        String fileName = "tree.txt";
        LessonTree[] lessons = new LessonTree[4];
        String[] source1 = {"2022", "CSE321", "LECTURE2"};
        String[] source2 = {"2022", "CSE321"};
        String[] source3 = {"2022", "CSE222"};
        String[] source4 = {"2023", "CSE232", "LECTURE2", "PROBLEM2"};
        String dest1 = "2023";
        String dest2 = "2020";
        String dest3 = "2020";
        String dest4 = "2022";
        Scanner scanner = new Scanner(System.in);
        

        try {
            for(int i = 0 ; i < lessons.length; ++i) {
                lessons[i] = new LessonTree();
                lessons[i].buildTree(fileName);
            }
        } catch (FileNotFoundException e) {
            System.out.println(e.toString());
        }

        lessons[0].displayTree();
        lessons[0].bfsSearch("CSE232");
        lessons[0].bfsSearch("CSE2332");

        lessons[0].dfsSearch("CSE232");
        lessons[0].dfsSearch("CSE2332");
        
        lessons[0].potSearch("CSE232");
        lessons[0].potSearch("CSE2332");

        System.out.println("For next part enter some character and press enter");
        scanner.nextLine();
        lessons[0].moveNode(source1, dest1);
        System.out.println("For next part enter some character and press enter");
        scanner.nextLine();
        lessons[1].moveNode(source2, dest2);
        System.out.println("For next part enter some character and press enter");
        scanner.nextLine();
        lessons[2].moveNode(source3, dest3);
        System.out.println("For next part enter some character and press enter");
        scanner.nextLine();
        lessons[3].moveNode(source4, dest4);
        scanner.close();
    }
}
