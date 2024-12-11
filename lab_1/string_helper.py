def normalizar_string(s):
    s = s.lower()
    s = (
        s.replace("á", "a")
        .replace("é", "e")
        .replace("í", "i")
        .replace("ó", "o")
        .replace("ú", "u")
    )
    s.strip()  # Quita los espacios del final (si hubiera)
    return s