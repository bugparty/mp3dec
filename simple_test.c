#include <stdio.h>
#include <stdlib.h>
#include "mp3frame.h"
#include "io.h"

int main(int argc, char *argv[])
{
    char *filename;
    int frame_count = 0;

    if(argc < 2) {
        filename = "res/320k.mp3";
        printf("No file specified, using default: %s\n", filename);
    } else {
        filename = argv[1];
    }

    printf("=== MP3 Frame Parser Test ===\n");
    printf("Opening file: %s\n\n", filename);

    io_open(filename, 1000000);

    printf("File size: %ld bytes\n", io_length());
    printf("\nParsing frames...\n");
    printf("----------------------------------------\n");

    while(frame_syncFrame()) {
        frame_count++;

        // Skip frame data to position at next frame
        // After frame_syncFrame(), we're at frame_start + 4 (after header)
        // Need to skip remaining (frameSize - 4) bytes
        int frame_size = frame_getFrameSize();
        io_seek(io_offset() + frame_size - 4);

        if(frame_count % 100 == 0) {
            frame_printStatus();
        }
    }

    printf("\n\n=== Parsing Complete ===\n");
    printf("Total frames parsed: %d\n", frame_count);
    printf("Track duration: %.2f seconds\n", frame_getDuration());
    printf("Bitrate: %d kbps\n", frame_get_bit_rate());
    printf("Sample rate: %d Hz\n", frame_get_sample_rate());
    printf("Channels: %d\n", frame_getChannels());

    io_close();

    return 0;
}
