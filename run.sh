#!/bin/bash

echo "Compilando..."

g++ main.cpp glad.c -Iinclude -lglfw -lGL -o app

if [ $? -eq 0 ]; then
    echo "Compilación exitosa 🚀"
    ./app
else
    echo "Error en compilación ❌"
fi