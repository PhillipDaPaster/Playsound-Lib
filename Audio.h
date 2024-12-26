#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <windows.h>
// phillips lib

class sound_player {
private:
    struct sound_data {
        std::vector<std::uint8_t> data;
        float volume;
    };

    std::unordered_map<std::uint8_t*, sound_data> sound_cache;

    void modify_volume(char* bytes, ptrdiff_t file_size, float volume) {
        int offset = 0;
        for (int i = 0; i < file_size - 4; i++) {
            if (bytes[i] == 'd' && bytes[i + 1] == 'a' && bytes[i + 2] == 't' && bytes[i + 3] == 'a') {
                offset = i;
                break;
            }
        }

        if (!offset) return;

        DWORD samples = *reinterpret_cast<DWORD*>(bytes + offset + 4) / 2;
        SHORT* sample = reinterpret_cast<SHORT*>(bytes + offset + 8);
        for (DWORD i = 0; i < samples; i++, sample++) {
            *sample = static_cast<SHORT>(*sample * volume);
        }
    }

public:
    void wav_to_memory(const std::string& file_name, BYTE** buffer, DWORD* file_size) {
        std::ifstream file(file_name, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open wav file: " + file_name);
        }

        file.seekg(0, std::ios::end);
        *file_size = file.tellg();
        *buffer = new BYTE[*file_size];
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(*buffer), *file_size);
    }

    void play_sound(uint8_t* bytes, size_t size, float volume) {
        auto& current = sound_cache[bytes];

        if (current.data.empty()) {
            current.data.resize(size);
        }

        if (current.volume != volume) {
            std::memcpy(current.data.data(), bytes, size);
            current.volume = volume;
            modify_volume(reinterpret_cast<char*>(current.data.data()), size, volume);
        }

        PlaySoundA(reinterpret_cast<char*>(current.data.data()), NULL, SND_ASYNC | SND_MEMORY);
    }

    ~sound_player() {
        for (auto& [key, sound_data] : sound_cache) {
            delete[] key;
        }
    }
};
