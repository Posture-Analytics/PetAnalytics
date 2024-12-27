import pandas as pd
import matplotlib.pyplot as plt

# Set style for a clean look
plt.style.use('seaborn-v0_8')
plt.rcParams['axes.grid'] = False

# Load the data
file_name = "labeled_data.csv"
data = pd.read_csv(file_name)

# Calculate distribution
activity_counts = data["label"].value_counts()

# Create figure
plt.figure(figsize=(10, 6), dpi=300)

# Create the main bar plot with a darker orange color
bars = plt.bar(activity_counts.index, activity_counts.values,
               color='#D35400',  # Darker orange
               alpha=0.85,
               width=0.7)

# Simple value labels
for bar in bars:
    height = bar.get_height()
    plt.text(bar.get_x() + bar.get_width()/2., height,
             f'{int(height)}',
             ha='center', va='bottom',
             fontsize=9)

# Customize the plot with minimal elements
plt.ylabel('Number of Samples', fontsize=11)

# Clean up x-axis labels
plt.xticks(rotation=45, ha='right', fontsize=10)
plt.yticks(fontsize=10)

# Show all spines (frame)
for spine in plt.gca().spines.values():
    spine.set_visible(True)
    spine.set_color('black')
    spine.set_linewidth(1)

# Set background to white
plt.gca().set_facecolor('white')
plt.gcf().set_facecolor('white')

# Adjust layout
plt.tight_layout()

# Save the plot
plt.savefig('activity_distribution.png', format='png', bbox_inches='tight', dpi=300)

# Show the plot
plt.show()
