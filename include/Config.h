//
// Created by alex on 10/7/20.
//

#ifndef SPHERE_VIS_CONFIG_H
#define SPHERE_VIS_CONFIG_H

#include <variant>
#include <glm/vec3.hpp>
#include <toml.hpp>

#ifdef __unix__
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#endif


struct Config
{
    int audio_sample_rate = 44100;
    int audio_frames_per_buffer = 128;
    float audio_amplify = 1.f;

    int sphere_rings;
    int sphere_sectors;

    glm::vec3 color_background{0.f};
    glm::vec3 color_foreground{0.6f, 0.19f, 0.8f};
};

struct Error
{
    std::string message;
};

std::variant<Config, Error> parse_config(const char* argument_file_name)
{
    std::string file_name;

    if(argument_file_name != nullptr)
    {
        file_name = argument_file_name;
    }
    else
    {
#ifdef __unix__
        const char* home_dir = nullptr;

        if((home_dir = getenv("HOME")) == nullptr)
        {
            home_dir = getpwuid(getuid())->pw_dir;
        }

        file_name = std::string(home_dir) + "/.config/sphere-vis/conf.toml";
#else
        return Error{"config file must be provided"};
#endif
    }

    auto data = toml::parse(file_name);

    Config config;

    if(data.contains("audio"))
    {
        const auto& audio = toml::find(data, "audio");

        if(audio.contains("sample_rate"))
            config.audio_sample_rate = toml::find<toml::integer>(audio, "sample_rate");

        if(audio.contains("frames_per_buffer"))
            config.audio_frames_per_buffer = toml::find<toml::integer>(audio, "frames_per_buffer");

        if(audio.contains("amplify"))
            config.audio_amplify = toml::find<toml::floating>(audio, "amplify");
    }

    if(data.contains("sphere"))
    {
        const auto& sphere = toml::find(data, "sphere");

        if(sphere.contains("rings"))
            config.sphere_rings = toml::find<toml::integer>(sphere, "rings");

        if(sphere.contains("sectors"))
            config.sphere_sectors = toml::find<toml::integer>(sphere, "sectors");
    }

    if(data.contains("colors"))
    {
        const auto& colors = toml::find(data, "colors");

        if(colors.contains("foreground"))
        {
            const auto& foreground = toml::find<std::vector<int>>(colors, "foreground");

            if(foreground.size() != 3)
                return Error{"incorrect value for colors.foreground"};

            config.color_foreground = glm::vec3(
                foreground[0] / 255.f,
                foreground[1] / 255.f,
                foreground[2] / 255.f
            );
        }

        if(colors.contains("background"))
        {
            const auto& background = toml::find<std::vector<int>>(colors, "background");

            if(background.size() != 3)
                return Error{"incorrect value for colors.background"};

            config.color_background = glm::vec3(
                    background[0] / 255.f,
                    background[1] / 255.f,
                    background[2] / 255.f
            );
        }
    }

    return config;
}

#endif //SPHERE_VIS_CONFIG_H
