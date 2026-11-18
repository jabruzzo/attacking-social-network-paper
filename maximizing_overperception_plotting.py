
import matplotlib.pyplot as plt
import pandas as pd

DATA_FILE_NAME = "maximizing_overperception_data.csv"
COLOR_DICT = {
	"greedy" : "green",
	"degree" : "red"
}
LABEL_DICT = {
	"greedy" : "Greedy Algorithm",
	"degree" : "Top-k Degree"
}
VARIABLE_LABEL_DICT = {
	"g" : "Objective Value",
	"time" : "Wall Clock Time"
}
VARIABLE_Y_AXIS_DICT = {
	"g" : "Fraction Overperceiving",
	"time" : "Time (sec)"
}


def read_data():

	df = pd.read_csv(DATA_FILE_NAME)

	return df


def main():

	d = read_data()

	d.columns = d.columns.str.strip()

	fig, axs = plt.subplots(2, 2, figsize=(10, 10), layout = "constrained")

	network_list = d["network"].unique()
	algo_list = d['algo'].unique()

	for i, network in enumerate(network_list):

		for j, variable in enumerate(['g', 'time']):

			for algo in algo_list:

				d_network_algo = d[(d['network'] == network) & (d['algo'] == algo)]

				axs[i, j].plot(d_network_algo['k'], d_network_algo[variable], color = COLOR_DICT[algo], label = LABEL_DICT[algo], linestyle='-', marker='o')
				axs[i, j].set_title(network + ", " + VARIABLE_LABEL_DICT[variable])
				axs[i, j].set_xlabel('k')
				axs[i, j].set_ylabel(VARIABLE_Y_AXIS_DICT[variable])
				axs[i, j].grid(True)

	handles, labels = axs[0,0].get_legend_handles_labels()

	fig.legend(
	    handles, 
	    labels, 
	    loc="outside center right"
	)

	plt.savefig('maximizing_overperception_results_plot.png', dpi=300, bbox_inches='tight')
	plt.show()


if __name__ == "__main__":
	main()