#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    uint32_t width;
    uint32_t height;
    float fps;
} VideoInfo;

const size_t FILE_NAME_LIMIT = FILENAME_MAX;
const size_t CAPTION_NAME_LIMIT = 200;
const size_t COMMAND_SIZE_LIMIT = 2000;

bool has_valid_file_extension(const char *filename);

char *get_filename_extension(const char *filename);

char *get_filename_without_extension(const char *filename);

void rename_file(char *filename, const char *new_name);

int countChar(char *str, char ch);

char **split_strings(char *string, char delimiter, int *outSize);

VideoInfo getVideoInfo(char *filename);


const char *video_formats[] = {"mp4",
                               "m4v",
                               "avi",
                               "wmv",
                               "mov",
                               "qt",
                               "mkv",
                               "webm",
                               "flv",
                               "gif"
};

const size_t VIDEO_FORMATS_LIST_SIZE = 10;

int main(int argc, char **argv) {
    if (argc < 2) {
        //help section.
        printf("BasicVidTimer. Patrick Keefe (c) 2023. \n");
        printf("This is a command line application. You must run it a terminal such as cmd, PowerShell, or Terminal and call %s in order for it to work.\n", argv[0]);
        printf("Usage:\n");
        printf("    -i          the input video file.\n");
        printf("    -o          the output video file.\n");
        printf("    -text       the signature for the video. use '\\' to create line breaks.\n");
        printf("    -USE_TIMER= specifies whether or not to display the video's elapsed time in the center of the screen. -USE_TIMER=TRUE will enable the timer.\n");
        printf("Press any key to continue...\n");
        getc(stdin);
        return 0;
    }

    char input_file_name[FILE_NAME_LIMIT];
    char output_file_name[FILE_NAME_LIMIT];
    char intermediate_file_name[FILE_NAME_LIMIT];
    char caption[CAPTION_NAME_LIMIT];

    memset(&input_file_name, 0, FILE_NAME_LIMIT);
    memset(&output_file_name, 0, FILE_NAME_LIMIT);

    bool use_timer = false;
    bool has_input = false;
    bool has_output = false;
    bool has_caption = false;

    printf("%s", *argv);
    printf("%i\n", argc);
    // check for both -i and -o.
    for (int i = 0; i < argc; ++i) {
        // read in args.
        char *currentArg = argv[i];
        // read in i.
        if (strcmpi(currentArg, "-i") == 0) {
            strncpy(input_file_name, argv[++i], FILE_NAME_LIMIT);
            if (input_file_name[0] == '\0') {
                fprintf(stderr, "Usage: -i [file name]\n");
                return -1;
            }
            has_input = true;

        }
        // read in -o
        if (strcmpi(currentArg, "-o") == 0) {
            strncpy(output_file_name, argv[++i], FILE_NAME_LIMIT);
            if (output_file_name[0] == '\0') {
                fprintf(stderr, "Usage: -o [file name]\n");
                return -1;
            }
            has_output = true;
        }
        // read in -text
        if (strcmpi(currentArg, "-text") == 0) {
            strncpy(caption, argv[++i], CAPTION_NAME_LIMIT);
            if (caption[0] == '\0') {
                fprintf(stderr, "Usage: -text [the caption you want in the video.]\n");
                return -1;
            }
            has_caption = true;
        }

        if (strncmp(currentArg, "-USE_TIMER", 10) == 0) {
            char *timer_flag_pos = currentArg + 11;
            if (strcmpi(timer_flag_pos, "TRUE") == 0) {
                use_timer = true;
            } else if (strcmpi(timer_flag_pos, "FALSE") != 0) {
                fprintf(stderr, "Invalid value for USE_TIMER. Usage: USE_TIMER=[TRUE/FALSE]\n");
                return -1;
            }
        }

    }
    if (!(has_input && has_output)) {
        fprintf(stderr, "You must specify an input and output file with -i and -o.\n");
        return -1;
    }

    if (!(use_timer || has_caption)) {
        if (strcmp(input_file_name, output_file_name) == 0) {
            printf("Input and output files have the same name and extension. Please give the output file a different name/extension.\n");
            return -1;
        } else {
            printf("Encoding video to new extension...\n");
            char re_encode_command[COMMAND_SIZE_LIMIT];
            sprintf(re_encode_command, ".\\ffmpeg.exe -loglevel error -y -i %s %s", input_file_name, output_file_name);
            system(re_encode_command);
        }
    } else {
        char *name = get_filename_without_extension(input_file_name);
        char *ext = get_filename_extension(output_file_name);

        strcpy(intermediate_file_name, name);
        strcat(intermediate_file_name, "_i");
        strcat(intermediate_file_name, ext);


        if (use_timer) {
            printf("Applying Timecode Reading...\n");
            VideoInfo vi = getVideoInfo(input_file_name);

            int margin_y = vi.height / 20.0f;

            char command[COMMAND_SIZE_LIMIT];

            sprintf(command,
                    ".\\ffmpeg.exe -loglevel error -y -i %s -vf \"drawtext=fontsize=34:timecode='00\\:00\\:00\\:00':r=%i:start_number=1:fontcolor=white:boxcolor=black:box=1:text='TCR\\:':boxborderw=20:x=(%i-tw)/2:y=%i\" -c:v h264 %s",
                    input_file_name,
                    (int) vi.fps,
                    vi.width,
                    margin_y,
                    intermediate_file_name);
            system(command);

        }
        if (has_caption) {
            printf("Applying Captions...\n");
            if (use_timer) {
                // reuse input.
                strcpy(input_file_name, intermediate_file_name);
            }

            // calculate the bounding box of the text.
            char command[COMMAND_SIZE_LIMIT];

            int string_count;
            char **strings = split_strings(caption, '\\', &string_count);
            if (!strings) {
                fprintf(stderr, "Caption was left empty.\n");
                return -1;
            }

            char draw_text_stack[COMMAND_SIZE_LIMIT];
            memset(draw_text_stack, 0, COMMAND_SIZE_LIMIT);

            int32_t margin = 35;
            int32_t text_height = 48;

            for (int i = 0; i < string_count; ++i) {
                char temp[200];
                char margin_x_temp[50];
                char margin_y_temp[50];

                sprintf(margin_x_temp, "x=%i:", margin);
                sprintf(margin_y_temp, "y=%i + (%i*%i)", margin, text_height, i);

                sprintf(temp, "drawtext=fontsize=32:text='%s':fontcolor=black:", strings[i]);
                strcat(temp, margin_x_temp);
                strcat(temp, margin_y_temp);
                // concatenate stack with temp.
                strcat(draw_text_stack, temp);
                if (i < string_count - 1) strcat(draw_text_stack, ",");
            }
            sprintf(command, ".\\ffmpeg.exe  -loglevel error -y -i %s -vf \"%s\" -c:v h264 %s",
                    input_file_name,
                    draw_text_stack,
                    output_file_name);
            system(command);

            // draw them all on the video, with vertical offsets of + 20.
            // do this by chaining multiple draw filters at once.


        }
        else
        {
            rename(intermediate_file_name, output_file_name);
        }

        remove(intermediate_file_name);

    }
    printf("Done!\n");
    return 0;
}

bool has_valid_file_extension(const char *filename) {
    char *last_period_loc = strrchr(filename, '.');
    if (!last_period_loc) return false;
    for (int i = 0; i < VIDEO_FORMATS_LIST_SIZE; ++i) {
        if (strcmp(video_formats[i], last_period_loc + 1) == 0)
            return true;
    }
    return false;
}

char *get_filename_extension(const char *filename) {
    //
    char *ext_portion = strrchr(filename, '.');
    size_t len = strlen(filename) - (size_t) (ext_portion - filename);

    char *extension = calloc(len + 1, 1);
    strncpy(extension, ext_portion, len);

    return extension;
}

char *get_filename_without_extension(const char *filename) {
    char *ext_portion = strrchr(filename, '.');
    size_t len = ext_portion - filename;

    char *extension = calloc(len + 1, 1);
    strncpy(extension, filename, len);

    return extension;
}

void rename_file(char *filename, const char *new_name) {
    char *ext = get_filename_extension(filename);
    if (strlen(new_name) + strlen(ext) > strlen(filename)) {
        size_t ext_len = strlen(ext);
        strncpy(filename, new_name, FILE_NAME_LIMIT - ext_len);
    }

    strcpy(filename, new_name);
    strcat(filename, ext);
    free(ext);
    // copy the name of the new_name string into filename.
}

char **split_strings(char *string, char delimiter, int *outSize) {
    if (!string || string[0] == '\0') return NULL;
    char **result = NULL;
    int strings_count = 1 + countChar(string, delimiter);
    result = calloc(1, strings_count * sizeof(char *));

    // get the string that comes up to the next occurrence of the character.
    char *delimiter_loc = string;
    char *current_loc = string;
    for (int i = 0; i < strings_count; ++i) {
        delimiter_loc = strchr(current_loc, delimiter);
        if (!delimiter_loc) delimiter_loc = string + strlen(string);
        // add anything

        // ignore the stupid symbol.
        ptrdiff_t diff = delimiter_loc - current_loc;
        char *new_string = calloc(diff + 1, 1);
        result[i] = strncpy(new_string, current_loc, diff);
        current_loc = delimiter_loc + 1;
        *outSize = i;
    }
    (*outSize)++;
    return result;

}

int countChar(char *str, char ch) {
    int count = 0;
    while (*str) {
        if (*str == ch) {
            count++;
        }
        str++;
    }
    return count;
}

VideoInfo getVideoInfo(char *filename) {
    VideoInfo result = {0};
    int a;
    //use ffprobe to print metadata.
    char command[COMMAND_SIZE_LIMIT];
    char output_text_name[FILE_NAME_LIMIT];
    char *name = get_filename_without_extension(filename);
    strcpy(output_text_name, name);
    strcat(output_text_name, "_md_res.txt");

    sprintf(command,
            ".\\ffprobe.exe -loglevel error -select_streams v:0 -show_entries stream=width,height -of csv=p=0 %s > %s",
            filename, output_text_name);
    system(command);

    FILE *fp = fopen(output_text_name, "r");
    char line[40];

    fgets(line, sizeof(line), fp);

    fclose(fp);
    // parse the line's text.
    char **nums = split_strings(line, ',', &a);
    result.width = strtol(nums[0], NULL, 10);
    result.height = strtol(nums[1], NULL, 10);

    free(nums[0]);
    free(nums[1]);

    remove(output_text_name);

    strcpy(output_text_name, name);
    strcat(output_text_name, "_md_fps.txt");

    sprintf(command,
            ".\\ffprobe.exe -loglevel error -select_streams v:0 -show_entries stream=avg_frame_rate -of csv=p=0 %s > %s",
            filename, output_text_name);
    system(command);

    fp = fopen(output_text_name, "r");

    fgets(line, sizeof(line), fp);

    fclose(fp);
    // parse the line's text.
    nums = split_strings(line, '/', &a);

    size_t numerator = strtol(nums[0], NULL, 10);
    size_t denominator = strtol(nums[1], NULL, 10);
    result.fps = (float) numerator / (float) denominator;

    free(nums[0]);
    free(nums[1]);


    remove(output_text_name);
    free(name);
    return result;


}