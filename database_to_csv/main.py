import csv
import pandas as pd
from utilities.label_extractor import extract_labels_meta
from utilities.imu_processor import process_imu_data
from utilities.data_merger import merge_data_with_labels

def main(data_json_file, labels_json_file):  # Removidos cutoff_time e cutoff_frame daqui
    """Função principal que coordena a execução."""
    try:
        label_data, meta_text_data = extract_labels_meta(labels_json_file)

        # Salvar meta_texts.csv (intervenção humana aqui)
        with open("meta_texts.csv", "w", newline="") as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(["start_frame", "meta_text"])
            writer.writerows(meta_text_data)
        print("meta_texts.csv gerado. Aguardando inserção manual de dados...")

        # Obter cutoff_time e cutoff_frame do usuário
        while True:
            cutoff_time = input("Digite o horário de corte (HH:MM:SS): ")
            try:
                pd.Timestamp(cutoff_time) #verifica se o input é um horario
                break  # Sai do loop se o formato for válido
            except ValueError:
                print("Formato de horário inválido. Use HH:MM:SS.")

        while True:
            try:
                cutoff_frame = int(input("Digite o frame de corte (inteiro): "))
                if cutoff_frame >= 0:
                    break
                else:
                    print("O frame de corte deve ser um número inteiro não negativo.")
            except ValueError:
                print("Entrada inválida. Digite um número inteiro.")

        df_imu = process_imu_data(data_json_file, cutoff_time, cutoff_frame)
        df_expanded = df_imu.explode('dados', ignore_index=True)
        df_expanded = pd.concat([df_expanded.drop(columns=['dados']), df_expanded['dados'].apply(pd.Series)], axis=1)

        df_final = merge_data_with_labels(df_expanded, label_data)
        df_final = df_final.dropna(axis=1, how='all')
        df_final.to_csv('labeled_data.csv', index=False)
        print("Arquivo labeled_data.csv gerado com sucesso!")

    except Exception as e:
        print(f"Ocorreu um erro: {e}")

if __name__ == "__main__":
    DATA_JSON_FILE = "data.json"
    LABELS_JSON_FILE = "labels.json"
    main(DATA_JSON_FILE, LABELS_JSON_FILE)  # Removidos os valores fixos
