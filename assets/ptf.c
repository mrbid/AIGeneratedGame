/*
    James William Fletcher (github.com/mrbid)
        October 2022 - October 2023

    Converts ASCII PLY (.ply) file to C OpenGL buffers.
    
    - Auto detects if index buffer is GLubyte, GLushort or GLuint.
    - Make sure meshes you export have been triangulated first,
      don't export UV coordinates only vertex, index, normal, and color.
    - This is not particularly fast. Not intended for large files.

    Compile: cc ptf.c -lm -Ofast -o ptf
    
    Usage: ./ptf filename_noextension
        (loads filenames from the 'ply/' directory)
    
    Example: ./ptf porygon
        (loads 'ply/porygon.ply' and outputs 'porygon.h' into the cwd)

    **********************************************************************
    An older version of this that uses GLuint for the index buffer
    and no fixed output buffers can be found here:
    https://github.com/esAux/esAux-Menger/blob/main/PTO/pto.c
    but this solution needs to be updated to use the %g format specifier
    and in some instances has the potential to garble file output.
    **********************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MAX_BUFF 1161326592 //387108864 //24194304 //6048576 //1048576

int main(int argc, char** argv)
{
    // start time
    time_t st = time(0);

    // ensure an input file is specified
    if(argc < 2)
    {
        printf("Please specify an input file.\n");
        return 0;
    }

    // take the input file name and strip any supplied extension
    char name[32] = {0};
    strcat(name, argv[1]);
    char* p = strstr(name, ".");
    if(p != NULL){*p = 0x00;}

    // generate the read file path (reads .ply files from a local `ply/` directory)
    char readfile[32] = {0};
    strcat(readfile, "ply/");
    strcat(readfile, name);
    strcat(readfile, ".ply");

    // pre-init our buffers
    char* vertex_array = calloc(1, MAX_BUFF);
    char* index_array = calloc(1, MAX_BUFF);
    char* normal_array = calloc(1, MAX_BUFF);
    char* color_array = calloc(1, MAX_BUFF);
    if(vertex_array == NULL || index_array == NULL || normal_array == NULL || color_array == NULL)
    {
        printf("Failed to allocate memory.\n");
        return 0;
    } 

    // open our ASCII PLY file for reading
    unsigned int numvert=0, numind=0, maxind=0;
    unsigned int mode = 0, mode2 = 0;
    printf("Open: %s\n", readfile);
    FILE* f = fopen(readfile, "r");
    while(f == NULL)
    {
        f = fopen(readfile, "r");
        sleep(1);
    }

    // do the conversion
    char line[256];
    char add[256];
    while(fgets(line, 256, f) != NULL)
    {
        //printf("%s\n",line);
        if(strcmp(line, "end_header\n") == 0)
        {
            mode = 1;
            continue;
        }

        // load index
        if(mode == 2)
        {
            unsigned int n,x,y,z;
            if(sscanf(line, "%u %u %u %u", &n, &x, &y, &z) == 4)
            {
                char add[256];
                sprintf(add, "%u,%u,%u,", x, y, z);
                strcat(index_array, add);
                if(x > maxind){maxind = x;}
                if(y > maxind){maxind = y;}
                if(z > maxind){maxind = z;}
                numind += 3;
            }
        }

        // load vertex, normal, color
        if(mode == 1)
        {
            float vx,vy,vz,nx,ny,nz,r,g,b;
            if((mode2 == 0 || mode2 == 1) && sscanf(line, "%f %f %f %f %f %f %f %f %f", &vx, &vy, &vz, &nx, &ny, &nz, &r, &g, &b) == 9)
            {
                sprintf(add, "%g,%g,%g,", vx, vy, vz);
                strcat(vertex_array, add);
                numvert++;

                sprintf(add, "%g,%g,%g,", nx, ny, nz);
                strcat(normal_array, add);

                sprintf(add, "%.3g,%.3g,%.3g,", 0.003921568859f*r, 0.003921568859f*g, 0.003921568859f*b);
                strcat(color_array, add);

                mode2 = 1;
            }
            else if((mode2 == 0 || mode2 == 2) && sscanf(line, "%f %f %f %f %f %f %f %f", &vx, &vy, &vz, &nx, &ny, &nz, &r, &g) == 8)
            {
                
                sprintf(add, "%g,%g,%g,", vx, vy, vz);
                strcat(vertex_array, add);
                numvert++;

                sprintf(add, "%g,%g,%g,", nx, ny, nz);
                strcat(normal_array, add);

                sprintf(add, "%g,%g,", r, -g); // flipping the V/T coordinate for OpenGL bottom up texture coordinates
                strcat(color_array, add);

                mode2 = 2;
            }
            else if((mode2 == 0 || mode2 == 3) && sscanf(line, "%f %f %f %f %f %f %f", &vx, &vy, &vz, &r, &g, &b, &nx) == 7)
            {
                sprintf(add, "%g,%g,%g,", vx, vy, vz);
                strcat(vertex_array, add);
                numvert++;

                sprintf(add, "%.3g,%.3g,%.3g,", 0.003921568859f*r, 0.003921568859f*g, 0.003921568859f*b);
                strcat(color_array, add);

                mode2 = 3;
            }
            else if((mode2 == 0 || mode2 == 4) && sscanf(line, "%f %f %f %f %f %f", &vx, &vy, &vz, &nx, &ny, &nz) == 6)
            {
                sprintf(add, "%g,%g,%g,", vx, vy, vz);
                strcat(vertex_array, add);
                numvert++;

                sprintf(add, "%g,%g,%g,", nx, ny, nz);
                strcat(normal_array, add);

                mode2 = 4;
            }
            else if((mode2 == 0 || mode2 == 5) && sscanf(line, "%f %f %f", &vx, &vy, &vz) == 3)
            {
                if(vx == 3.0 && vy == 0.0 && vz == 1.0)
                {
                    strcat(index_array, "0,1,2,");
                    numind += 3;
                    mode = 2;
                    continue;
                }

                sprintf(add, "%g,%g,%g,", vx, vy, vz);
                strcat(vertex_array, add);
                numvert++;

                mode2 = 5;
            }
            else
            {
                strcat(index_array, "0,1,2,");
                numind += 3;
                mode = 2;
                continue;
            }
        }

    }
    
    // close PLY file
    fclose(f);
    
    // remove trailing comma's
    if(vertex_array[0] != 0x00)
        vertex_array[strlen(vertex_array)-1] = 0x00;
    if(normal_array[0] != 0x00)
        normal_array[strlen(normal_array)-1] = 0x00;
    if(index_array[0] != 0x00)
        index_array[strlen(index_array)-1] = 0x00;
    if(color_array[0] != 0x00)
        color_array[strlen(color_array)-1] = 0x00;

    // output the resultant file
    char outfile[256];
    sprintf(outfile, "%s.h", name);
    f = fopen(outfile, "w");
    while(f == NULL)
    {
        f = fopen(outfile, "w");
        sleep(1);
    }

    fprintf(f, "\n#ifndef %s_H\n#define %s_H\n\nconst GLfloat %s_vertices[] = {%s};\n", name, name, name, vertex_array);
    if(normal_array[0] != 0x00)
        fprintf(f, "const GLfloat %s_normals[] = {%s};\n", name, normal_array);
    if(color_array[0] != 0x00)
    {
        if(mode2 == 2)
            fprintf(f, "const GLfloat %s_uvmap[] = {%s};\n", name, color_array);
        else
            fprintf(f, "const GLfloat %s_colors[] = {%s};\n", name, color_array);
    }
    
    if(maxind <= 255)
    {
        fprintf(f, "const GLubyte %s_indices[] = {%s};\nconst GLsizeiptr %s_numind = %u;\nconst GLsizeiptr %s_numvert = %u;\n\n#endif\n", name, index_array, name, numind, name, numvert);
        printf("Output: %s.h (UBYTE)\nMODE: %u\n", name, mode2);
    }
    else if(maxind <= 65535)
    {
        fprintf(f, "const GLushort %s_indices[] = {%s};\nconst GLsizeiptr %s_numind = %u;\nconst GLsizeiptr %s_numvert = %u;\n\n#endif\n", name, index_array, name, numind, name, numvert);
        printf("Output: %s.h (USHORT)\nMODE: %u\n", name, mode2);
    }
    else
    {
        fprintf(f, "const GLuint %s_indices[] = {%s};\nconst GLsizeiptr %s_numind = %u;\nconst GLsizeiptr %s_numvert = %u;\n\n#endif\n", name, index_array, name, numind, name, numvert);
        printf("Output: %s.h (UINT)\nMODE: %u\n", name, mode2);
    }

    fclose(f);
    const float mins = ((float)(time(0)-st))/60.f;
    if(mins > 0.001f){printf("Time Taken: %.2f mins\n", mins);}
    return 0;
}