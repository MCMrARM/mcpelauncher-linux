#pragma once

class MinecraftCommands;

class Minecraft {

public:

    static MinecraftCommands* (*Minecraft_getCommands)(Minecraft*);

    MinecraftCommands* getCommands() {
        return Minecraft_getCommands(this);
    }

};