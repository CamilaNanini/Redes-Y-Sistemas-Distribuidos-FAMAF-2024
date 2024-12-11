import requests
import random
from string_helper import normalizar_string

def test_pelis_del_genero():
    genero = "accion"  # Género a probar
    response = requests.get(f"http://localhost:5000/peliculas/{genero}")
    assert response.status_code == 200
    for pelicula in response.json():
        assert normalizar_string(pelicula["genero"]) == genero
    print("Paso correctamente test_pelis_del_genero")


def test_pelis_con_cierto_string():
    palabra = "J"
    response = requests.get(f"http://localhost:5000/peliculas/buscar/{palabra}")
    assert response.status_code == 200
    for pelicula in response.json():
        assert palabra.lower() in pelicula["titulo"].lower()
    print("Paso correctamente test_pelis_con_cierto_string")


def test_peli_aleatoria():
    response = requests.get("http://localhost:5000/peliculas/aleatoria")
    assert response.status_code == 200
    assert response.json() is not None
    print("Paso correctamente test_peli_aleatoria")

def test_peli_aleatoria_segun_genero():
    generos = ["accion", "ciencia ficcion", "aventura", "drama"]

    for genero in generos:
        response = requests.get(f"http://localhost:5000/peliculas/aleatoria/{genero}")
        assert response.status_code == 200
        assert normalizar_string(response.json()["genero"]) == genero

    print("Paso correctamente test_peli_aleatoria_segun_genero")

def test_peli_en_feriado():
    generos = ["accion", "ciencia ficcion", "aventura", "drama"]
    tipos = ["puente", "inamovible" ,"trasladable", "nolaborable"]
    for genero in generos:
         for tipo in tipos:
            response = requests.get(f"http://localhost:5000/peliculas/feriado/{genero}/{tipo}")
            assert response.status_code == 200
            assert response.json() is not None
    print("Paso correctamente test_peli_en_feriado")



test_pelis_del_genero()
test_pelis_con_cierto_string()
test_peli_aleatoria()
test_peli_aleatoria_segun_genero()
test_peli_en_feriado()
print("Todos los tests pasaron correctamente.")
