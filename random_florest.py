import pandas as pd
from sklearn.model_selection import train_test_split, GridSearchCV
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report, accuracy_score

# Carregar os dados
file_name = "labeled_data.csv"
data = pd.read_csv(file_name)

#data = data[~data["label"].isin(["Standing", "Eating"])]  # Remover as classes "Standing" e "Sitting"

# Separar features e labels
X = data[["aX", "aY", "aZ", "gX", "gY", "gZ"]]  # Seleciona apenas as colunas com os valores
y = data["label"]

# Dividir os dados em treino e teste
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=44)

# Definir os hiperparâmetros para validação cruzada
param_grid = {
    "max_depth": [5, 10, 15],
    "n_estimators": [100, 250, 400]
}

# Criar o modelo
rf = RandomForestClassifier(random_state=42)

# Configurar o GridSearchCV
grid_search = GridSearchCV(
    estimator=rf,
    param_grid=param_grid,
    cv=5,
    scoring="accuracy",
    n_jobs=-1
)

# Treinar o modelo com validação cruzada
grid_search.fit(X_train, y_train)

# Exibir os melhores hiperparâmetros
print("Melhores hiperparâmetros:", grid_search.best_params_)

# Avaliar o modelo no conjunto de teste
best_rf = grid_search.best_estimator_
y_pred = best_rf.predict(X_test)

# Exibir os resultados
print("\nRelatório de Classificação:\n", classification_report(y_test, y_pred))
print("Acurácia no conjunto de teste:", accuracy_score(y_test, y_pred))