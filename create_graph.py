import pandas as pd
import matplotlib.pyplot as plt

# Carregar os dados do CSV
file_name = "labeled_data.csv"
data = pd.read_csv(file_name)

# Contar o número de amostras para cada atividade
activity_counts = data["label"].value_counts()

# Criar o gráfico de barras
plt.figure(figsize=(10, 6))
plt.bar(activity_counts.index, activity_counts.values, color="blue", alpha=0.7)

# Adicionar títulos e rótulos
plt.title("Activity", fontsize=16)
plt.xlabel("Activity", fontsize=12)
plt.ylabel("Number of Samples", fontsize=12)
plt.xticks(rotation=45, fontsize=10)
plt.tight_layout()

# Exibir o gráfico
plt.show()
