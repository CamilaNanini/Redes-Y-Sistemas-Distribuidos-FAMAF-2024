Con respecto al Laboratorio 0:
Existe una branch en este mismo repositorio que contiene el trabajo de los tres integrates.

Funcionalidades ...
-Ver el listado de películas: /peliculas
-Agregar una película: /peliculas
-Actulizar una película: /peliculas/<int:id>
-Eliminar una película: /peliculas/<int:id>
-Obtener una pelicula aleatoria: /peliculas/aleatoria
-Obtener todas las películas de un género: /peliculas/<string:genero>
-Obtener las peliculas con una pablabra en su título:/peliculas/buscar/<string:palabra>
-Obtener una pelicula aleatoria según un género: /peliculas/aleatoria/<string:genero>
-Obtener una película aleatoria para el próximo feriado del tipo indicado: /peliculas/feriado/<string:genero>/<string:tipo>

Tener en cuenta: Las palabras que se quieran agregar a las urls deben ser en minúsculas, sin tildes ni espacio.

Presentación: https://docs.google.com/presentation/d/1dN7HS5gFq6XQEBdXL3nEHfVoUP8oUbQQbSnbvZR-3r0/edit?usp=sharing
