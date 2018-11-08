import serial, time
import plotly.offline as py
import plotly.graph_objs as go

ser = serial.Serial('COM3', 9600, timeout=1, write_timeout=1)

def read_lines(): #  Reading from the serial, sends text to activate it
    ser.write("send".encode())
    d = ser.readlines()
    return d[1:]

def get_values(d):
    processed = []
    for val in d:
        val = str(val)
        val = val.replace("b'","").replace("\\r","").replace("\\n'","") # Replaces every non-number character with nothing
        val = float(val)/4 # Converts the bytes (n*4) back to normal
        processed.append(val)
    return processed

def filter_values(altitude):
    filtered = []
    for i in range(3,497):
        filtered.append((altitude[i-2]+altitude[i-1]+altitude[i]+altitude[i+1]+altitude[i+2])/5) # Average of 5 values
    return filtered

def get_apogee(altitude):
    apo = max(altitude)
    return apo, altitude.index(apo)*0.05

def get_max_speed(altitude):
    maxDelta = -1
    for i in range(9, 489):
        if abs(altitude[i-5]-altitude[i+5])>maxDelta:
            maxDelta = abs(altitude[i-5]-altitude[i+5])
            maxSpeedIndex = i
    maxSpeed = maxDelta/1000*3600*20/10
    return maxSpeed, maxSpeedIndex

def create_graph(processed, apo, maxSpeed, title):
    time_values = []
    for i in range(len(processed)):
        time_values.append(i*0.05)

    trace = go.Scatter(
        x = time_values,
        y = processed
    )
    data = [trace]

    layout = go.Layout(
        showlegend = False,
        annotations = [
            dict(
                x=apo[1],
                y=apo[0],
                xref='x',
                yref='y',
                text='Apogee: %fm'%apo[0],
                showarrow=True,
                arrowhead=7,
                ax=0,
                ay=-40
            ),
            dict(
                x=maxSpeed[1]*0.05,
                y=processed[maxSpeed[1]],
                xref='x',
                yref='y',
                text='Max speed: %fkm/h'%maxSpeed[0],
                showarrow=True,
                arrowhead=7,
                ax=0,
                ay=-40
            ),
        ]
    )
    
    fig = go.Figure(data=data, layout=layout)
    py.plot(fig, filename='%s.html' % title)

def get_data(title):
    d = read_lines()
    processed = get_values(d)
    print(processed)
    print("\n")
    filtered = filter_values(processed)
    print(filtered)
    print("\n")
    apo = get_apogee(filtered)
    print(apo)
    maxSpeed = get_max_speed(filtered)
    print(maxSpeed)
    print("\nGenerating chart...")
    create_graph(filtered,apo,maxSpeed,title)


filename = str(input("Enter the file name (.html): "))
get_data(filename)
