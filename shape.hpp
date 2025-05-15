#include "constants.hpp"

#define SHAPE_WIDTH 3


struct Shape
{
    int count;
    bool positions[SHAPE_WIDTH][SHAPE_WIDTH][SHAPE_WIDTH];

    void rotate(Axis, Transformation);
    Shape getRotated(Axis, Transformation);

    int getLowestIndex();

    bool getPositionAt(Axis a, unsigned int p, unsigned int q, unsigned int r)
    {
        if (a == AXIS_X) return positions[r][p][q];
        else if (a == AXIS_Y) return positions[p][r][q];
        else return positions[p][q][r];
    }
    void setPositionAt(Axis a, unsigned int p, unsigned int q, unsigned int r, bool val)
    {
        if (a == AXIS_X) positions[r][p][q] = val;
        else if (a == AXIS_Y) positions[p][r][q] = val;
        else positions[p][q][r] = val;
    }
};

void Shape::rotate(Axis axis, Transformation rd)
{
    if (rd != ROT_CCW && rd != ROT_CW)
        return;

    for (int r = 0; r < SHAPE_WIDTH; r++) // r - axis on which rotation is being performed
    {
        bool arr[SHAPE_WIDTH][SHAPE_WIDTH] = { 0 }; // current XZ matrix
        for (int p = 0; p < SHAPE_WIDTH; p++)
            for (int q = 0; q < SHAPE_WIDTH; q++)
                arr[p][q] = getPositionAt(axis, p, q, r);

        // Transpose matrix
        bool tarr[SHAPE_WIDTH][SHAPE_WIDTH] = { 0 };
        for (int p = 0; p < SHAPE_WIDTH; p++)
            for (int q = 0; q < SHAPE_WIDTH; q++)
                tarr[p][q] = arr[q][p];
        
        if (rd == ROT_CCW)
        {
            // Reverse each *row* of transposed matrix
            for (int p = 0; p < SHAPE_WIDTH; p++)
                for (int q = 0; q < SHAPE_WIDTH; q++)
                    arr[p][q] = tarr[p][SHAPE_WIDTH - q - 1];
        }
        else if (rd == ROT_CW)
        {
            // Reverse each *column* of transposed matrix
            for (int p = 0; p < SHAPE_WIDTH; p++)
                for (int q = 0; q < SHAPE_WIDTH; q++)
                    arr[p][q] = tarr[SHAPE_WIDTH - p - 1][q];
        }

        // Update positions
        for (int p = 0; p < SHAPE_WIDTH; p++)
            for (int q = 0; q < SHAPE_WIDTH; q++)
                setPositionAt(axis, p, q, r, arr[p][q]);
    }
}

Shape Shape::getRotated(Axis axis, Transformation rd)
{
    // REVIEW: pretpostavlja se validni rd

    Shape rotated;
    rotated.count = this->count;

    for (int r = 0; r < SHAPE_WIDTH; r++) // r - axis on which rotation is being performed
    {
        bool arr[SHAPE_WIDTH][SHAPE_WIDTH] = { 0 }; // current XZ matrix
        for (int p = 0; p < SHAPE_WIDTH; p++)
            for (int q = 0; q < SHAPE_WIDTH; q++)
                arr[p][q] = getPositionAt(axis, p, q, r);

        // Transpose matrix
        bool tarr[SHAPE_WIDTH][SHAPE_WIDTH] = { 0 };
        for (int p = 0; p < SHAPE_WIDTH; p++)
            for (int q = 0; q < SHAPE_WIDTH; q++)
                tarr[p][q] = arr[q][p];
        
        if (rd == ROT_CCW)
        {
            // Reverse each *row* of transposed matrix
            for (int p = 0; p < SHAPE_WIDTH; p++)
                for (int q = 0; q < SHAPE_WIDTH; q++)
                    arr[p][q] = tarr[p][SHAPE_WIDTH - q - 1];
        }
        else if (rd == ROT_CW)
        {
            // Reverse each *column* of transposed matrix
            for (int p = 0; p < SHAPE_WIDTH; p++)
                for (int q = 0; q < SHAPE_WIDTH; q++)
                    arr[p][q] = tarr[SHAPE_WIDTH - p - 1][q];
        }

        // Update positions
        for (int p = 0; p < SHAPE_WIDTH; p++)
            for (int q = 0; q < SHAPE_WIDTH; q++)
                rotated.setPositionAt(axis, p, q, r, arr[p][q]);
    }

    return rotated;
}

// Get y-index of lowest block in Shape
int Shape::getLowestIndex()
{
    for (int j = 0; j < SHAPE_WIDTH; j++)
        for (int i = 0; i < SHAPE_WIDTH; i++)
            for (int k = 0; k < SHAPE_WIDTH; k++)
                if (positions[i][j][k])
                    return j;

    return SHAPE_WIDTH - 1; // REVIEW: ovo je postavljeno jer inače bude warning da nema return; svakako bi loop uvik treba nać indeks
}

Shape shapes[] = {
    {
        // straight / I
        .count = 3,
        .positions = 
        {
            {   // x = 0
                { 0, 0, 0 },   // y = 0
                { 0, 0, 0 },   // y = 1
                { 0, 0, 0 }    // y = 2
            },
            {   // x = 1
                { 0, 0, 0 },   // y = 0
                { 1, 1, 1 },   // y = 1
                { 0, 0, 0 }    // y = 2
            },
            {   // x = 2
                { 0, 0, 0 },   // y = 0
                { 0, 0, 0 },   // y = 1
                { 0, 0, 0 }    // y = 2
            }
        }
    },
    {
        // V
        .count = 3,
        .positions = 
        {
            {   // x = 0
                { 0, 0, 0 },   // y = 0
                { 0, 0, 0 },   // y = 1
                { 0, 0, 0 }    // y = 2
            },
            {   // x = 1
                { 0, 0, 0 },   // y = 0
                { 1, 1, 0 },   // y = 1
                { 0, 1, 0 }    // y = 2
            },
            {   // x = 2
                { 0, 0, 0 },   // y = 0
                { 0, 0, 0 },   // y = 1
                { 0, 0, 0 }    // y = 2
            }
        }
    },
    {
        // L
        .count = 4,
        .positions = 
        {
            {   // x = 0
                { 0, 0, 0 },   // y = 0
                { 0, 0, 0 },   // y = 1
                { 0, 0, 0 }    // y = 2
            },
            {   // x = 1
                { 1, 1, 0 },   // y = 0
                { 0, 1, 0 },   // y = 1
                { 0, 1, 0 }    // y = 2
            },
            {   // x = 2
                { 0, 0, 0 },   // y = 0
                { 0, 0, 0 },   // y = 1
                { 0, 0, 0 }    // y = 2
            }
        }
    },
    {
        // T
        .count = 4,
        .positions = 
        {
            {   // x = 0
                { 0, 0, 0 },   // y = 0
                { 0, 0, 0 },   // y = 1
                { 0, 0, 0 }    // y = 2
            },
            {   // x = 1
                { 0, 0, 0 },   // y = 0
                { 1, 1, 1 },   // y = 1
                { 0, 1, 0 }    // y = 2
            },
            {   // x = 2
                { 0, 0, 0 },   // y = 0
                { 0, 0, 0 },   // y = 1
                { 0, 0, 0 }    // y = 2
            }
        }
    },
    {
        // stair / Z
        .count = 4,
        .positions = 
        {
            {   // x = 0
                { 0, 0, 0 },   // y = 0
                { 0, 0, 0 },   // y = 1
                { 0, 0, 0 }    // y = 2
            },
            {   // x = 1
                { 0, 0, 0 },   // y = 0
                { 0, 1, 1 },   // y = 1
                { 1, 1, 0 }    // y = 2
            },
            {   // x = 2
                { 0, 0, 0 },   // y = 0
                { 0, 0, 0 },   // y = 1
                { 0, 0, 0 }    // y = 2
            }
        }
    },
    {
        // chiral A
        .count = 4,
        .positions = 
        {
            {   // x = 0
                { 0, 0, 0 },   // y = 0
                { 0, 0, 0 },   // y = 1
                { 0, 1, 0 }    // y = 2
            },
            {   // x = 1
                { 0, 0, 0 },   // y = 0
                { 1, 1, 0 },   // y = 1
                { 0, 1, 0 }    // y = 2
            },
            {   // x = 2
                { 0, 0, 0 },   // y = 0
                { 0, 0, 0 },   // y = 1
                { 0, 0, 0 }    // y = 2
            }
        }
    },
    {
        // chiral B
        .count = 4,
        .positions = 
        {
            {   // x = 0
                { 0, 0, 0 },   // y = 0
                { 0, 0, 0 },   // y = 1
                { 0, 0, 0 }    // y = 2
            },
            {   // x = 1
                { 0, 0, 0 },   // y = 0
                { 1, 1, 0 },   // y = 1
                { 0, 1, 0 }    // y = 2
            },
            {   // x = 2
                { 0, 0, 0 },   // y = 0
                { 0, 0, 0 },   // y = 1
                { 0, 1, 0 }    // y = 2
            }
        }
    },
    {
        // axes
        .count = 4,
        .positions = 
        {
            {   // x = 0
                { 0, 0, 0 },   // y = 0
                { 0, 0, 0 },   // y = 1
                { 0, 0, 0 }    // y = 2
            },
            {   // x = 1
                { 0, 0, 0 },   // y = 0
                { 1, 1, 0 },   // y = 1
                { 0, 1, 0 }    // y = 2
            },
            {   // x = 2
                { 0, 0, 0 },   // y = 0
                { 0, 1, 0 },   // y = 1
                { 0, 0, 0 }    // y = 2
            }
        }
    },
};