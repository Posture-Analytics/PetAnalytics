import pandas as pd

def merge_data_with_labels(df_expanded, label_data):
    """Realiza o merge dos dados expandidos com as labels."""
    df_labels = pd.DataFrame([{"frame": frame, "label": label} for start, end, label in label_data for frame in range(start, end + 1)])
    df_merged = pd.merge(df_expanded, df_labels, on="frame", how="left")
    df_final = df_merged[df_merged["label"].notnull()]
    return df_final
