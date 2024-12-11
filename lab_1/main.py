from operator import ne
from flask import Flask, jsonify, request
import random
from string_helper import normalizar_string
import proximo_feriado

app = Flask(__name__)
peliculas = [
    {"id": 1, "titulo": "Indiana Jones", "genero": "Acción"},
    {"id": 2, "titulo": "Star Wars", "genero": "Acción"},
    {"id": 3, "titulo": "Interstellar", "genero": "Ciencia ficción"},
    {"id": 4, "titulo": "Jurassic Park", "genero": "Aventura"},
    {"id": 5, "titulo": "The Avengers", "genero": "Acción"},
    {"id": 6, "titulo": "Back to the Future", "genero": "Ciencia ficción"},
    {"id": 7, "titulo": "The Lord of the Rings", "genero": "Fantasía"},
    {"id": 8, "titulo": "The Dark Knight", "genero": "Acción"},
    {"id": 9, "titulo": "Inception", "genero": "Ciencia ficción"},
    {"id": 10, "titulo": "The Shawshank Redemption", "genero": "Drama"},
    {"id": 11, "titulo": "Pulp Fiction", "genero": "Crimen"},
    {"id": 12, "titulo": "Fight Club", "genero": "Drama"},
]

# --Request handling--


def obtener_peliculas():
    return jsonify(peliculas)


def obtener_pelicula(id):
    # Lógica para buscar la película por su ID y devolver sus detalles
    for pelicula_actual in peliculas:
        if pelicula_actual["id"] == id:
            return jsonify(pelicula_actual)
    return jsonify({"mensaje": "Pelicula no encontrada"}), 404


def agregar_pelicula():
    nueva_pelicula = {
        "id": obtener_nuevo_id(),
        "titulo": request.json["titulo"],
        "genero": request.json["genero"],
    }
    peliculas.append(nueva_pelicula)
    return jsonify(nueva_pelicula), 201


def actualizar_pelicula(id):
    # Lógica para buscar la película por su ID y actualizar sus detalles
    for pelicula_actual in peliculas:
        if pelicula_actual["id"] == id:
            pelicula_actual["titulo"] = request.json["titulo"]
            pelicula_actual["genero"] = request.json["genero"]
            return jsonify(pelicula_actual), 200

    return jsonify({"mensaje": "Pelicula no encontrada"}), 404


def eliminar_pelicula(id):
    # Lógica para buscar la película por su ID y eliminarla
    for pelicula_a_borrar in peliculas:
        if pelicula_a_borrar["id"] == id:
            peliculas.remove(pelicula_a_borrar)
            return jsonify({"mensaje": "Película eliminada correctamente"})
    return jsonify({"mensaje": "La película no fue eliminada correctamente"}), 404


def obtener_nuevo_id():
    if len(peliculas) > 0:
        ultimo_id = peliculas[-1]["id"]
        return ultimo_id + 1
    else:
        return 1


def pelis_del_genero(genero):
    # Devulve el listado de películas de un género específico
    genero = normalizar_string(genero)
    lista_de_pelis = []
    for pelicula_actual in peliculas:
        if normalizar_string(pelicula_actual["genero"]) == genero:
            lista_de_pelis.append(pelicula_actual)
    return jsonify(lista_de_pelis), 200


def pelis_con_cierto_string(palabra):
    # Devuelven la lista de películas que tengan determinado string en el título
    lista_de_pelis = []
    for pelicula_actual in peliculas:
        if palabra.lower() in pelicula_actual["titulo"].lower():
            lista_de_pelis.append(pelicula_actual)
    return jsonify(lista_de_pelis), 200


def peli_aleatoria():
    # Devuelven una pelicula aleatoria
    peli_random = random.choice(peliculas)
    return jsonify(peli_random), 200


def peli_aleatoria_segun_genero(genero):
    # Devuelven una pelicula aleatoria pero de un género especifico
    genero = normalizar_string(genero)
    lista_de_pelis = []
    for pelicula_actual in peliculas:
        if normalizar_string(pelicula_actual["genero"]) == genero:
            lista_de_pelis.append(pelicula_actual)
    if lista_de_pelis != []:
        peli_random = random.choice(lista_de_pelis)
        return jsonify(peli_random), 200
    else:
        return jsonify({}), 404


def peli_en_feriado(genero, tipo):
    # Objeto de la clase auxiliar que handlea la api de feriados
    next_holiday = proximo_feriado.NextHoliday()
    next_holiday.next_holiday_type(tipo)

    if next_holiday.holiday == None:
        print(f"WARNING:next_holiday was none")  # Borrar despues de debuggear
        return jsonify({"mensaje": "Ese tipo de feriado no existe"}), 404

    genero = normalizar_string(genero)  # Para simplificarr la busqueda
    lista_de_pelis = []  # Para tomar una al azar de esta lista

    for pelicula_actual in peliculas:
        if normalizar_string(pelicula_actual["genero"].lower()) == genero:
            lista_de_pelis.append(pelicula_actual)

    if lista_de_pelis != []:
        # 'Empacando' la response
        peli_random = random.choice(lista_de_pelis)
        feriado = next_holiday.holiday
        response = {"pelicula": peli_random, "feriado": feriado}
        return jsonify(response), 200

    else:  # Si no habia pelis que coincidan con el genero
        return jsonify({}), 404


# -------------------------------


app.add_url_rule("/peliculas", "obtener_peliculas", obtener_peliculas, methods=["GET"])
app.add_url_rule(
    "/peliculas/<int:id>", "obtener_pelicula", obtener_pelicula, methods=["GET"]
)
app.add_url_rule("/peliculas", "agregar_pelicula", agregar_pelicula, methods=["POST"])
app.add_url_rule(
    "/peliculas/<int:id>", "actualizar_pelicula", actualizar_pelicula, methods=["PUT"]
)
app.add_url_rule(
    "/peliculas/<int:id>", "eliminar_pelicula", eliminar_pelicula, methods=["DELETE"]
)

app.add_url_rule(
    "/peliculas/aleatoria", "peli_aleatoria", peli_aleatoria, methods=["GET"]
)
app.add_url_rule(
    "/peliculas/<string:genero>", "pelis_del_genero", pelis_del_genero, methods=["GET"]
)
app.add_url_rule(
    "/peliculas/buscar/<string:palabra>",
    "pelis_con_cierto_string",
    pelis_con_cierto_string,
    methods=["GET"],
)
app.add_url_rule(
    "/peliculas/aleatoria/<string:genero>",
    "peli_aleatoria_segun_genero",
    peli_aleatoria_segun_genero,
    methods=["GET"],
)
app.add_url_rule(
    "/peliculas/feriado/<string:genero>/<string:tipo>",
    "peli_en_feriado",
    peli_en_feriado,
    methods=["GET"],
)

if __name__ == "__main__":
    app.run()
