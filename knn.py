import pandas as pd
from sklearn.model_selection import train_test_split, GridSearchCV
from sklearn.neighbors import KNeighborsClassifier
from sklearn.metrics import classification_report, accuracy_score

# Carregar os dados
file_name = "labeled_data.csv"
data = pd.read_csv(file_name)

# Separar features e labels
X = data[["aX", "aY", "aZ", "gX", "gY", "gZ"]]  # Seleciona apenas as colunas com os valores
y = data["label"]

# Dividir os dados em treino e teste
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Definir os hiperparâmetros para validação cruzada
param_grid = {
    "n_neighbors": [5],  # Deve ser o núemero de comportamentos
    "weights": ["uniform", "distance"],  # Pesos para os vizinhos
    "metric": ["euclidean", "manhattan", "minkowski"]  # Métricas de distância
}

# Criar o modelo
knn = KNeighborsClassifier()

# Configurar o GridSearchCV
grid_search = GridSearchCV(
    estimator=knn,
    param_grid=param_grid,
    cv=5,
    scoring="accuracy"
)

# Treinar o modelo com validação cruzada
grid_search.fit(X_train, y_train)

# Exibir os melhores hiperparâmetros
print("Melhores hiperparâmetros:", grid_search.best_params_)

# Avaliar o modelo no conjunto de teste
best_knn = grid_search.best_estimator_
y_pred = best_knn.predict(X_test)

# Exibir os resultados
print("\nRelatório de Classificação:\n", classification_report(y_test, y_pred))
print("Acurácia no conjunto de teste:", accuracy_score(y_test, y_pred))
