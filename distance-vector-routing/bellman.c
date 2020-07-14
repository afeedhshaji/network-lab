#include <stdio.h>
#include <stdlib.h>

#define MAX 100

void Bellman_Ford(int G[MAX][MAX], int S, int V, int E, int edge[MAX][2])
{
    int i, u, v, k, distance[MAX], parent[MAX], flag = 1;
    for (i = 0; i < V; i++)
        distance[i] = 1000, parent[i] = -1;
    distance[S] = 0;
    for (i = 0; i < V - 1; i++)
    {
        for (k = 0; k < E; k++)
        {
            u = edge[k][0], v = edge[k][1];
            if (distance[u] + G[u][v] < distance[v])
                distance[v] = distance[u] + G[u][v], parent[v] = u;
        }
    }
    for (k = 0; k < E; k++)
    {
        u = edge[k][0], v = edge[k][1];
        if (distance[u] + G[u][v] < distance[v])
        {
            printf("%d ", distance[v]);
        }
    }

    printf("Distance Vector Routing Table for router %d \n__________________________________________\n\n", S);
    for (int i = 0; i < V; i++)
        printf("%d \t \t %d\n", i, distance[i]);

    printf("\n\n");
    return;
}
int main()
{
    int V, edge[MAX][2], G[MAX][MAX], i, j, k = 0;
    printf("BELLMAN FORD\n");
    printf("Enter no. of vertices: ");
    scanf("%d", &V);
    printf("Enter graph in matrix form:\n");
    for (i = 0; i < V; i++)
        for (j = 0; j < V; j++)
        {
            scanf("%d", &G[i][j]);
            if (G[i][j] != 0)
                edge[k][0] = i, edge[k++][1] = j;
        }
    for (int i = 0; i < V; i++)
    {
        Bellman_Ford(G, i, V, k, edge);
    }

    return 0;
}