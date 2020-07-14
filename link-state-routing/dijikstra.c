#include <stdio.h>
#include <limits.h>
#include <stdbool.h>

#define MAX 100

int minDistance(int dist[], int V, bool sptSet[])
{

    int min = INT_MAX, min_index;

    for (int v = 0; v < V; v++)
        if (sptSet[v] == false && dist[v] <= min)
            min = dist[v], min_index = v;

    return min_index;
}
int printSolution(int dist[], int V, int src)
{
    printf("Link State Routing Table for router %d \n____________________________________\n\n", src);
    for (int i = 0; i < V; i++)
        printf("%d \t \t %d\n", i, dist[i]);

    printf("\n\n");
}

void dijkstra(int graph[MAX][MAX], int V, int src)
{
    int dist[V];

    bool sptSet[V];

    for (int i = 0; i < V; i++)
        dist[i] = INT_MAX, sptSet[i] = false;

    // Distance of source vertex from itself is always 0
    dist[src] = 0;

    // Find shortest path for all vertices
    for (int count = 0; count < V - 1; count++)
    {

        int u = minDistance(dist, V, sptSet);

        sptSet[u] = true;

        for (int v = 0; v < V; v++)

            if (!sptSet[v] && graph[u][v] && dist[u] != INT_MAX && dist[u] + graph[u][v] < dist[v])
                dist[v] = dist[u] + graph[u][v];
    }

    printSolution(dist, V, src);
}

int main()
{
    int V, edge[MAX][2], G[MAX][MAX], i, j = 0;
    printf("DIJIKSTRA\n");
    printf("Enter no. of vertices: ");
    scanf("%d", &V);
    printf("Enter graph in matrix form:\n");
    for (i = 0; i < V; i++)
        for (j = 0; j < V; j++)
        {
            scanf("%d", &G[i][j]);
        }
    for (int i = 0; i < V; i++)
    {
        dijkstra(G, V, i);
    }
    return 0;
}