#include <raylib.h>
#include <math.h>
#include <string.h>
#include <csoundlib.h>
#include <stdio.h>

/* each segment of plot is (PLOT_WIDTH) / (SEGMENT_WIDTH * MAX_VALUES) */

#define PLOT_HEIGHT    450   
#define PLOT_WIDTH     1000   
#define RECT_WIDTH     4
#define SEGMENT_WIDTH  (RECT_WIDTH+1)
#define MAX_VALUES     (PLOT_WIDTH / SEGMENT_WIDTH)   
#define FFT_LEN        1024


float RMS = 0.0;
Color DEFAULT_COLOR = SKYBLUE;
double FFTG_BUFFER[FFT_LEN];

int saturate_int(int val, int max, int min) 
{
    if (val > max) return max; 
    else if (val < min) return min;
    else return val;
}

double get_mag(double real, double imag) 
{
    return sqrt(real*real + imag*imag);
}

double get_log(double val) 
{
    return log10f((float)val);
}

Color get_color_from_fft_value(double val)
{
    Color color = DEFAULT_COLOR;
    color.r = saturate_int(color.r + val*20, 255, 0);
    color.g = saturate_int(color.g - val*20, 255, 0);
    return color;
}

void master_output_vu(int trackId,
        unsigned char* input_buffer, 
        size_t len, 
        CSL_DTYPE data_type, 
        CSL_SR sample_rate,
        size_t num_channels) 
{
    float f_buffer[2000];
    double d_buffer[2000];
    double fftc_buffer[2000];
    double fftr_buffer[2000];
    int samps = byte_buffer_to_float_buffer(input_buffer, f_buffer, len, data_type);
    float new_rms = 0.0;
    for (int i = 0; i < samps; i++) 
    {
        new_rms += f_buffer[i] * f_buffer[i];
        d_buffer[i] = (double)f_buffer[i];
    }
    new_rms /= samps;
    RMS = envelope_follower(sqrtf(new_rms), 0.001, 0.05, RMS);
    int num_fts = rfft_forward_1d_array(d_buffer, FFT_LEN*2, samps, 1, 1.0, fftc_buffer);
    if (num_fts) 
    {
        for (int i = 0; i < FFT_LEN; i++)
        {
            fftr_buffer[i] = get_log(get_mag(fftc_buffer[i*2], fftc_buffer[i*2 + 1]));
        }
        memcpy(FFTG_BUFFER, fftr_buffer, FFT_LEN * sizeof(double));
    }
}

void start_sound(void)
{
    int ret;
    ret = soundlib_start_session(SR48000, CSL_S32);
    int ind = soundlib_get_default_input_device_index();

    ret = soundlib_start_input_stream(ind, 0.01);

    int oind = soundlib_get_default_output_device_index();

    ret = soundlib_start_output_stream(oind, 0.01);

    soundlib_set_master_volume(-100.0);

    soundlib_add_track(0);
    soundlib_register_input_ready_callback(0, &master_output_vu);
}

int main(void)
{
    start_sound();
    InitWindow(PLOT_WIDTH, PLOT_HEIGHT, "Audio Plot");

    float amp_values[MAX_VALUES] = {0}; // Array to store amplitude values
    double spec_values[MAX_VALUES][FFT_LEN] = {{0}, {0}}; // 2d array to store MAX_VAL number of FFT arrays
    int current_index = 0;           // Index for the latest value
    float time_elapsed = 0;          // Time elapsed for adding values
    int total_values = 0;            // Total number of values generated
    bool show_settings = false;

    Font custom_font = LoadFont("./Inria_Sans/InriaSans-Regular.ttf");

    SetTargetFPS(60); 
    
    while (!WindowShouldClose())
    {
        time_elapsed += GetFrameTime();
        if (time_elapsed > 0.001f) // Add a new value every N seconds
        {
            time_elapsed = 0;
            amp_values[current_index] = 1000 * RMS; 
            memcpy(spec_values[current_index], FFTG_BUFFER, FFT_LEN);

            current_index = (current_index + 1) % MAX_VALUES; // Loop around if we reach the max
            total_values++; // Increment total values generated
        }

        if (IsKeyPressed(KEY_S))
        {
            show_settings = true;
        }
        if (IsKeyPressed(KEY_ENTER))
        {
            show_settings = false;
        }

        BeginDrawing();
        ClearBackground(DEFAULT_COLOR);

        /* draw audio visualization */
        for (int i = 0; i < MAX_VALUES; i++)
        {
            int index = (current_index - MAX_VALUES + i + MAX_VALUES) % MAX_VALUES;
            int x = i * SEGMENT_WIDTH; 
            int rectHeight = (int)fabs(amp_values[index]); 
            int y = PLOT_HEIGHT / 2 - rectHeight / 2; 
            for (int i = 0; i < FFT_LEN; i++) 
            {
                DrawRectangle(x, (PLOT_HEIGHT) - i*4, SEGMENT_WIDTH, 4, get_color_from_fft_value(spec_values[index][i]));
            }
            Color rec_col = DARKGRAY;
            rec_col.a = 100;
            DrawRectangle(x, y, RECT_WIDTH, rectHeight, rec_col);
        }

        /* draw settings */
        if (show_settings == false) 
        {
            DrawTextEx(custom_font, "S = Settings", (Vector2){35, 35}, 22, 2, BLACK);
        }
        else 
        {
            Color settings_color = (Color){.r = DARKGRAY.r, .g = DARKGRAY.g, .b = DARKGRAY.b, 150};
            DrawRectangle(35, 35, 250, 300, settings_color);
            DrawTextEx(custom_font, "Settings", (Vector2){125 + 5, 35+10}, 15, 2, BLACK);
        }
        EndDrawing();
    }

    CloseWindow(); 
    return 0;
}