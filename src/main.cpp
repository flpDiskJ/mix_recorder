#include <stdio.h>
using namespace std;
#include <SDL.h>
#include <string>
#include <iostream>
#include <unistd.h>
#include <vector>
#include "AudioFile.h"

const int SAMPLE_RATE = 32000;
const int SAMPLE_CHUNK = 2048;
const unsigned int MAX_RECORD_MINUTES = 120;

bool update = true;
Sint16 channel_max = 0;

SDL_Rect level;

SDL_AudioSpec recording_spec;

AudioFile<int> audioFile;

unsigned int sample_count = 0;
float recording_seconds = 0;
unsigned int recording_minutes = 0;

int recording_device_count = 0;

void recording_callback(void* userdata, Uint8* stream, int len)
{
  sample_count += len/4;

  channel_max = 0;

  Sint16 val_1 = 0, val_2 = 0;

  for (int i = 0; i < len; i += 4)
    {
      val_1 = (stream[i+1] & 0xFF) << 8 | (stream[i] & 0xFF);
      val_2 = (stream[i+3] & 0xFF) << 8 | (stream[i+2] & 0xFF);
      audioFile.samples[0].push_back(val_1);
      audioFile.samples[1].push_back(val_2);
      if (val_1 > channel_max)
	{
	  channel_max = val_1;
	}
      if (val_2 > channel_max)
	{
	  channel_max = val_2;
	}
    }
  if (channel_max < 0) {channel_max = 0;}
  level.h = channel_max / 128;
  level.y = 300 - level.h;
  update = true;
  
  if (sample_count > SAMPLE_RATE * 15)
    {
      recording_seconds += (float)sample_count / (float)SAMPLE_RATE;
      sample_count = 0;
      if (recording_seconds >= 60.0)
	{
	  recording_seconds = 0;
	  recording_minutes++;
	}
      cout << recording_minutes << "Min " << (int)recording_seconds << "Sec\n";
    }
}

int main()
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
      cout << "SDL init failed" << SDL_GetError() << "\n";
      return 1;
    }

  const size_t cwd_size = 1024;
  char cwd_path[cwd_size];
  if (getcwd(cwd_path, cwd_size) == NULL)
    {
        // If _getcwd returns NULL, print an error message
        cerr << "Error getting current working directory" << endl;
    }

  level.w = 100;
  level.h = 5;
  level.x = 100;
  level.y = 300 - 5;

  audioFile.setNumChannels(2);
  audioFile.setBitDepth(16);
  audioFile.setSampleRate(SAMPLE_RATE);

  SDL_AudioDeviceID rec_id = 0;
  SDL_AudioSpec return_spec;

  SDL_zero(recording_spec);
  recording_spec.freq = SAMPLE_RATE;
  recording_spec.format = AUDIO_S16;
  recording_spec.channels = 2;
  recording_spec.samples = SAMPLE_CHUNK;
  recording_spec.callback = recording_callback;

  recording_device_count = SDL_GetNumAudioDevices(SDL_TRUE);

  if (recording_device_count < 1)
    {
      cout << "No recording devices available!\n";
      return 2;
    }

  int index;

  for (int i = 0; i < recording_device_count; ++i)
    {
      const char* device_name = SDL_GetAudioDeviceName(i, SDL_TRUE);
      cout << i << " - " << device_name << "\n";
    }

  cout << "Enter number for desired audio device\n";
  scanf("%d", &index);

  rec_id = SDL_OpenAudioDevice(SDL_GetAudioDeviceName(index, SDL_TRUE), SDL_TRUE, &recording_spec, &return_spec, 0);

  if (rec_id == 0)
    {
      cout << "Failed to open audio device!\n";
	return 3;
    }

  cout << "Enter filename\n";
  string file_name;
  cin >> file_name;

  SDL_Window *window = SDL_CreateWindow("Mix Recording", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 300, 300, SDL_WINDOW_SHOWN);

  if (window == NULL)
    {
      cout << "Window isn't working!\n";
      return 1;
    }

  SDL_Renderer *render = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

  if (render == NULL)
    {
      cout << "Renderer not working!\n";
      return 1;
    }
  
  //////////////////////////////////////////////////////

  SDL_PauseAudioDevice(rec_id, SDL_FALSE);

  string full_file_name;
  
  SDL_Event e;
  bool record = true;
  bool hault = false;
  unsigned int file_count = 0;
  while (!hault)
    {
      cout << "Recording...  ";
      cout << "esc or q to quit / s to split recording\n";
      while (record)
	{
	  while (SDL_PollEvent(&e))
	    {
	      switch (e.type)
		{
		case SDL_QUIT:
		  hault = true;
		  record = false;
		  break;
	      
		case SDL_KEYDOWN:
		  switch (e.key.keysym.sym)
		    {
		    case SDLK_ESCAPE:
		    case SDLK_q:
		      hault = true;
		      record = false;
		      break;

		    case SDLK_s:
		      record = false;
		      cout << "Splitting file...\n";
		      break;
		  
		    default:
		      break;
		    }
		  break;

		case SDL_WINDOWEVENT:
		  update = true;
		  break;
	      
		default:
		  break;
		}
	    }

	  if (update)
	    {
	      update = false;
	  
	      SDL_SetRenderDrawColor(render, 0, 0, 0, 0xFF);
	      SDL_RenderClear(render);
	      SDL_SetRenderDrawColor(render, 0, 150, 0, 0xFF);
	      SDL_RenderDrawLine(render, 20, 90, 280, 90);
	      SDL_RenderDrawLine(render, 20, 91, 280, 91);
	      SDL_SetRenderDrawColor(render, 200, 200, 0, 0xFF);
	      SDL_RenderDrawLine(render, 20, 70, 280, 70);
	      SDL_RenderDrawLine(render, 20, 71, 280, 71);

	      if (level.h < 210)
		{
		  SDL_SetRenderDrawColor(render, 0, 200, 0, 0xFF); // green
		} else if (level.h < 240)
		{
		  SDL_SetRenderDrawColor(render, 200, 200, 0, 0xFF); // yellow
		} else {
		SDL_SetRenderDrawColor(render, 200, 0, 0, 0xFF); // red
	      }
	      SDL_RenderFillRect(render, &level);
	      
	      SDL_SetRenderDrawColor(render, 200, 0, 0, 0xFF);
	      SDL_RenderDrawLine(render, 20, 50, 280, 50);
	      SDL_RenderDrawLine(render, 20, 51, 280, 51);
	  
	      SDL_RenderPresent(render);
	      if (recording_minutes >= MAX_RECORD_MINUTES)
		{
		  cout << "Max Recording Time Reached!\n";
		  record = false;
		}
	    }
      
	}
      if (record == false)
	{
	  cout << "Saving File...\n";

	  full_file_name.clear();
	  full_file_name += cwd_path;
	  full_file_name += '/' + file_name + to_string(file_count) + ".wav";
	  cout << full_file_name << "\n";

	  SDL_PauseAudioDevice(rec_id, SDL_TRUE);
	  audioFile.save(full_file_name, AudioFileFormat::Wave);
	  audioFile.clearAudioBuffer();
	  audioFile.setNumChannels(2);
	  SDL_PauseAudioDevice(rec_id, SDL_FALSE);

	  recording_minutes = 0;
	  recording_seconds = 0;
	  record = true;
	  file_count++;
	}
    }
  
  SDL_CloseAudioDevice(rec_id);
  SDL_DestroyRenderer(render);
  SDL_DestroyWindow(window);
  SDL_Quit();
  
  return 0;
}
