import requests
from datetime import date
from string_helper import normalizar_string

def get_url(year):
    return f"https://nolaborables.com.ar/api/v2/feriados/{year}"

months = ['Enero','Febrero','Marzo','Abril','Mayo','Junio','Julio','Agosto','Septiembre','Octubre','Noviembre','Diciembre']
days = ['Domingo', 'Lunes', 'Martes', 'Miércoles', 'Jueves', 'Viernes', 'Sábado']

def day_of_week(day, month, year):
    return days[date(year, month, day).weekday()]

class NextHoliday:
    def __init__(self):
        self.loading = True
        self.year = date.today().year
        self.holiday = None

    def set_next(self, holidays): 
        now = date.today()
        today = {
            'day': now.day,
            'month': now.month
        }

        holiday = next(
            (h for h in holidays if h['mes'] == today['month'] and h['dia'] > today['day'] or h['mes'] > today['month']),
            holidays[0]
        )

        self.loading = False
        self.holiday = holiday

    def next_holiday_type(self,tipo):
        """
        Carga el iterador @holiday solamente con feriados del tipo 'tipo', si el tipo no es valido
        retorna None
        """
        response = requests.get(get_url(self.year))
        holidays = response.json()

        now = date.today()
        today = {
            'day': now.day,
            'month': now.month
        }

        tipo = normalizar_string(tipo)
        print(tipo)
        if tipo not in ["puente", "inamovible" ,"trasladable", "nolaborable"]:
            return None
        
        todosdeltipo=[]
        for h in holidays:
            if normalizar_string(h['tipo']) == tipo:
                todosdeltipo.append(h)
        if todosdeltipo == []:
            print("No hay feriados de ese tipo")
            return

        holiday = next(
            (h for h in todosdeltipo if h['mes'] == today['month'] and h['dia'] > today['day'] or h['mes'] > today['month']),
            todosdeltipo[0]
        )

        self.loading = False
        self.holiday = holiday
        

    def fetch_holidays(self): 
        response = requests.get(get_url(self.year))
        data = response.json()
        self.set_next(data)

    def render(self):
        if self.loading:
            print("Buscando...")
        else:
            print("Próximo feriado")
            print(self.holiday['motivo'])
            print("Fecha:")
            print(day_of_week(self.holiday['dia'], self.holiday['mes'] - 1, self.year))
            print(self.holiday['dia'])
            print(months[self.holiday['mes'] - 1])
            print("Tipo:")
            print(self.holiday['tipo'])

next_holiday = NextHoliday()
next_holiday.fetch_holidays()
next_holiday.render()
