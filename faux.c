#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_SIZE 10000000


int rotate(char * img, char ** output, int * length) {
    //there are at least 54 bytes in the header.
    //dynamic memory allocation for header.

    *output = malloc(MAX_SIZE * sizeof(char));
    if (!*output) {
        perror("Could not allocate output buffer");
        return 1;
    }
    int out_index = 0;

    unsigned char *header=malloc(54*sizeof(unsigned char));

    //checking if memory allocation succeeded.
    if (!header){
        perror("Error: Memory allocation failed.\n");
        return 1;
    } 

    //reading BMP file's header.
    unsigned int file_size,offset,pic_size,width,height,check,i,old_padding;
    memcpy(header, img, sizeof(char) * 54); // fread(header,sizeof(unsigned char),54,stdin);

    //checking if the first 2 bytes represent the 'B','M' characters, because that's
    //how we can recognize if the file is a BMP file.
    if (!(header[0]=='B' && header[1]=='M')){
        perror("Error: Not a BMP file.\n");
        free(*output);
        free(header);
        return 1;
    }

    //checking if 24 bits are used for colour represation.(that piece of information is contained in the 28th byte of the header)
    if (*(unsigned *)&header[28]!=24){
        perror ("Only 24-bit BMP images supported\n");
        free(*output);
        free(header);
        return 1;
    }

    //collecting data that is needed from the header.
    file_size = *(unsigned *)&header[2];  
    offset = *(unsigned *)&header[10];
    width = *(unsigned *)&header[18];
    height = *(unsigned *)&header[22];
    pic_size=*(unsigned char *)&header[34];
    old_padding=(4-(3*width)%4)%4;

    //checking if the BMP file follows the Windows 3.x format, as required, which is understood from the pictures offset.
    if (offset<54){
        perror ("BMP file does not follow Windows 3.x format.\n");
        free(*output);
        free(header);
        return 1;
    }
    
    //swapping the height and width and making height the new width and width the new height.
    unsigned int new_width = height;
    unsigned int new_height = width;
    //calculating the rotated picture's padding.
    unsigned int new_padding = (4-(3*new_width)%4)%4;
    //calculating the rotated picture's file size 
    unsigned int new_file_size = offset + new_height*(3*new_width+new_padding);
    unsigned int new_pic_size = new_height*(3*new_width+new_padding);

    *(unsigned int *)&header[2]=new_file_size;
    *(unsigned int *)&header[18]=new_width;
    *(unsigned int *)&header[22]=new_height;
    *(unsigned int *)&header[34]=new_pic_size;
    //extracting header data to stdout.
    for(int i = 0 ; i < 54; i++) {
        (*output)[out_index++] = header[i];
    }
    // dynamically allocating memory to store and then print other data of picture to stdout.
    unsigned char *other=malloc((offset-54)*sizeof(unsigned char));
    //checking if memory allocation succeeded.
    if (!other){
        perror("Memory allocation ERROR.\n");
        free(*output);
        free(header);
        free(other);
        return 1;
    }
    
    //reading and exctracting extra data unchanged.
    memcpy(other, img + 54, sizeof(char) * (offset - 54)); // fread (other,sizeof(unsigned char),offset-54,stdin);
    for(int i = 0 ; i < offset - 54; i++) {
        (*output)[out_index++] = other[i];
    }


    //dynamic memory allocation for an 1-D array to store the bytes that represent the picture's pixels.
    unsigned char *pixels = malloc ((file_size-offset)*sizeof(unsigned char));
    //checking if memory allocation succeeded.
    if (!pixels){
        perror("Memory allocation ERROR.\n");
        free(*output);
        free(header);
        free(other);
        return 1;
    }
    //reading the pixel data.
    memcpy(pixels, img + offset, sizeof(char) * pic_size);
    
    unsigned int new_index,j;
    //dynamically allocating and initializing an array with 0 with the size of the rotated picture's padding.
    unsigned char *new_padding_array = calloc (new_padding,sizeof(unsigned char)); // new image padding
    
    for (j=width ; j>0 ; j--){      // start the process from the end of the picture's end.
        new_index=3*(j-1);      //calculate the byte in the first row that we want to be printed to stdout first.
        for (i=0 ; i<height ; i++){
            for(int i = 0 ; i < 3; i++) {
                (*output)[out_index++] = pixels[new_index+i];
            }
            new_index+=3*width+old_padding;      // add to the index the number of bytes needed to get to byte exactly on the next row under the one we were at.
        }
        for(int i = 0 ; i < new_padding; i++) {
            (*output)[out_index++] = new_padding_array[i];
        }
    }

    //freeing the allocated memory. 
    free(other);
    free (pixels); 
    free (new_padding_array);
    free (header);
    *length = out_index;
    return 0;

}

int main (void){

    char * img = malloc(MAX_SIZE * sizeof(char));
    if (!img) {
        perror("Could not allocate image");
        exit(1);
    }
    fread(img, sizeof(char), MAX_SIZE, stdin);
    char * output;
    int length;
    rotate(img, &output, &length);
    for(int i = 0 ; i < length; i++) {
        putchar(output[i]);
    }
    free(output);
    //exit code for successful execution of the program.
    return 0;
}

