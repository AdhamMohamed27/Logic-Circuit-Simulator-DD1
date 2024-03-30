import matplotlib.pyplot as plt

# Function to read simulation output file
def read_simulation_file(file_path):   #The file path is the .sim file that the main program produces
    timestamps = []
    variables = {}

    with open(file_path, 'r') as file:
        for line in file:
            parts = line.strip().split(',')
            timestamp = int(parts[0])
            variable = parts[1]
            value = int(parts[2])

            if variable not in variables:
                variables[variable] = {'timestamps': [], 'values': []}

            variables[variable]['timestamps'].append(timestamp)
            variables[variable]['values'].append(value)

            if timestamp not in timestamps:
                timestamps.append(timestamp)

    return timestamps, variables

# Function to plot waveforms
def plot_waveforms(timestamps, variables):
    num_plots = len(variables)
    fig, axs = plt.subplots(num_plots, 1, figsize=(10, 6*num_plots))

    colors = ['blue', 'green', 'red', 'orange', 'purple']  # Add more colors if needed

    for i, (variable, data) in enumerate(variables.items()):
        axs[i].step(data['timestamps'], data['values'], label=variable, where='post', color=colors[i % len(colors)])
        axs[i].set_xlabel('Time')
        axs[i].set_ylabel('Value')
        axs[i].set_title(f'Waveform for {variable}')
        axs[i].grid(True)
        axs[i].legend()

    plt.tight_layout()
    plt.subplots_adjust(hspace=0.5)  # Adjust vertical space between subplots
    plt.show()

# Main function
def main():
    file_path = "D:\spring 24\DD1 project\Logic-Circuit-Simulator-DD1\Test_Cases\Test_Case_2\T2.sim"
    timestamps, variables = read_simulation_file(file_path)
    plot_waveforms(timestamps, variables)

if __name__ == "__main__":
    main()
