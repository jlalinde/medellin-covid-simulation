#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <math.h>
#include <cstdio>

#include "grid.h"


//
// initialize grid and fill it with particles
// 
void grid_init(grid_t & grid, int size)
{
    grid.size = size;

    // Initialize grid
    grid.grid = (linkedlist**) malloc(sizeof(linkedlist*) * size * size);

    if (grid.grid == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory for the grid!\n");
        exit(1);
    }

    memset(grid.grid, 0, sizeof(linkedlist*) * size * size);

    // Initialize locks
    grid.lock = (omp_lock_t*) malloc(sizeof(omp_lock_t) * size * size);

    if (grid.lock == NULL)
    {
        fprintf(stderr, "Error: Could not allocate memory for the locks!\n");
        exit(2);
    }

    for (int i = 0; i < size*size; ++i)
    {
        omp_init_lock(&grid.lock[i]);
    }
}

//
// adds a particle pointer to the grid
//
void grid_add(grid_t & grid, particle_t * p)
{
    int gridCoord = grid_coord_flat(grid.size, p->x, p->y);

    linkedlist_t * newElement = (linkedlist_t *) malloc(sizeof(linkedlist));
    newElement->value = p;

    // Beginning of critical section
    omp_set_lock(&grid.lock[gridCoord]);
    newElement->next = grid.grid[gridCoord];

    grid.grid[gridCoord] = newElement;
    // End of critical section
    omp_unset_lock(&grid.lock[gridCoord]);
}

//
// Removes a particle from a grid
//
bool grid_remove(grid_t & grid, particle_t * p, int gridCoord)
{
    if (gridCoord == -1)
        gridCoord = grid_coord_flat(grid.size, p->x, p->y);

    // No elements?
    if (grid.grid[gridCoord] == 0)
    {
        return false;
    }

    omp_set_lock(&grid.lock[gridCoord]);

    linkedlist_t ** nodePointer = &(grid.grid[gridCoord]);
    linkedlist_t * current = grid.grid[gridCoord];

    while(current && (current->value != p))
    {
        nodePointer = &(current->next);
        current = current->next;
    }

    if (current)
    {
        *nodePointer = current->next;
        free(current);
    }

    omp_unset_lock(&grid.lock[gridCoord]);
    return !!current;
}

//
// clears a grid from values and deallocates any memory from the heap.
//
void grid_clear(grid_t & grid)
{
    for (int i = 0; i < grid.size*grid.size; ++i)
    {
        linkedlist_t * curr = grid.grid[i];
        while(curr != 0)
        {
            linkedlist_t * tmp = curr->next;
            free(curr);
            curr = tmp;
        }
    }
    free(grid.grid);
}

int grid_size(grid_t & grid)
{
    int count = 0;
    for (int i = 0; i < grid.size * grid.size; ++i)
    {
        linkedlist_t * curr = grid.grid[i];
        while(curr != 0)
        {
            count++;
            curr = curr->next;
        }
    }
    return count;
}
