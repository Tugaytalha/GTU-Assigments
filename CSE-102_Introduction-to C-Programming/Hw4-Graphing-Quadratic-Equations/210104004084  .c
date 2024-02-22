#include <stdio.h>

int make_file(int a, int b, int c)
{ // This functing create a file and enter the coefficients
    FILE *me;
    me = fopen("coefficients.txt", "w");
    fprintf(me, "%d\n%d\n%d\n", a, b, c);
    fclose(me);
    printf("File succesfullly created\n");
    return 1;
}

int main()
{
    int x, y, cas, a, b, c, i, n, m;
    FILE *me, *graph;

    while (1)
    {
        printf(
            "\n\nPlease selcet your operation\n"
            "0. Enter the coefficients\n"
            "1. Draw the graph\n"
            "2. Print the graph into a txt file  \n"
            "3. Exit.\n"
            "------------------------------\n");
        printf("\nYOUR CHOICE: ");
        scanf("%d", &cas);

        switch (cas)
        {
        case 0:
            printf(
                "\nPlease enter three numbers for following equation: x = a . y^2 + b . y + a\n"
                "a : ");

            scanf("%d", &a);
            printf(
                "b : ");

            scanf("%d", &b);
            printf(
                "c : ");

            scanf("%d", &c);

            make_file(a, b, c);
            break;
        case 1:

            me = fopen("coefficients.txt", "r");
            if (!(me))
            {
                printf("File doesn't exist please create file first\n");
                break;
            }
            fscanf(me, "%d", &a); //taking coefficients
            fscanf(me, "%d", &b);
            fscanf(me, "%d", &c);
            printf("Coefficients has been read from the coefficients.txt file\n");
            fclose(me);

            printf("Printing the graph of %d*(y*y) + %d*y + %d\n", a, b, c);

            for (i = 0; i < 56; i++)
                printf(" ");
            printf("^\n");
            for (y = 15; y >= -15; y--)
            {
                if (y == 0)
                    printf("<");
                else
                    printf(" ");

                n = a * (y * y) + b * y + c;
                for (x = -55; x <= 55; x++)
                {
                    if (x == n)
                    {
                        printf("\033[0;34m"); // Set the text to the color blue
                        printf("#");
                        printf("\033[0m"); // reset color
                    }
                    else if (y == -1 && (x % 10 == 7 || x % 10 == -3) && x != 0 && ((x + 1) != n && (x + 2) != n)) 
                    {
                        printf("\033[5;33m"); // Set the text to the color yellow-brown dimming
                        printf("%3d", x + 3);
                        printf("\033[0m"); // reset color
                        x += 2;
                    }
                    else if (x == -4 && y % 5 == 0 && y != 0 && ((x + 1) != n && (x + 2) != n))
                    {
                        printf("\033[5;33m"); // Set the text to the color yellow-brown dimming
                        printf("%3d", y);
                        printf("\033[0m"); // reset color
                        x += 2;
                    }
                    else if (x == -1 && y == -1)
                    {
                        printf("\033[5;33m"); // Set the text to the color yellow-brown dimming
                        printf("%.1d", 0);
                        printf("\033[0m"); // reset color
                    }
                    else if (x == 0) // y axis
                        printf("|");
                    else if (y == 0) // x axis
                        printf("-");
                    else
                        printf(" ");
                }
                if (y == 0)
                    printf(">");
                printf("\n");
            }
            for (i = 0; i < 56; i++)
                printf(" ");
            printf("v\n");
            break;
        case 2:
            me = fopen("coefficients.txt", "r");
            if (!(me))
            {
                printf("File doesn't exist please create file first\n");
                break;
            }
            fscanf(me, "%d", &a); //taking coefficients
            fscanf(me, "%d", &b);
            fscanf(me, "%d", &c);
            printf("Coefficients has been read from the coefficients.txt file\n");
            printf("%d %d %d\n", a, b, c);
            fclose(me);
            
            graph = fopen("graph.txt","w"); //for writing graph

            for (i = 0; i < 56; i++)
                fprintf(graph, " ");
            fprintf(graph, "^\n");
            for (y = 15; y >= -15; y--)
            {
                // fprintf(graph,"%d", y);
                if (y == 0)
                    fprintf(graph, "<");
                else
                    fprintf(graph, " ");

                n = a * (y * y) + b * y + c;
                for (x = -55; x <= 55; x++)
                {
                    if (x == n)
                    {
                        fprintf(graph, "#");
                    }
                    else if (x == 0)
                        fprintf(graph, "|");
                    else if (y == 0)
                        fprintf(graph, "-");
                    else
                        fprintf(graph, " ");
                }
                if (y == 0)
                    fprintf(graph, ">");
                fprintf(graph, "\n");
            }
            for (i = 0; i < 56; i++)
                fprintf(graph, " ");
            fprintf(graph, "v\n");
            fclose(graph);
            printf("The graph of %d*(y*y) + %d*y + %d  has been written to graph.txt file.\n", a, b, c);
            break;
        case 3:
            printf("Exitting....\n");
            return 0;

        default:
            printf("Error! operator is not correct\n\n");
        }
    }
}