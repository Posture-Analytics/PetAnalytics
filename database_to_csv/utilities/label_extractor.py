import json

def extract_labels_meta(labels_json_file):
    """Extrai labels e meta textos de um arquivo JSON de labels."""
    try:
        with open(labels_json_file, "r") as file:
            data = json.load(file)
    except (FileNotFoundError, json.JSONDecodeError) as e:
        raise Exception(f"Erro ao carregar JSON de labels: {e}")

    label_data = []
    meta_text_data = []

    for annotation in data[0]["annotations"]:
        for result in annotation["result"]:
            start = result["value"]["ranges"][0]["start"]
            end = result["value"]["ranges"][0]["end"]
            label = result["value"]["timelinelabels"][0]

            meta_text = result.get("meta", {}).get("text", [])
            if meta_text:
                meta_text_data.append([start, meta_text[0]])
            else:
                label_data.append([start, end, label])

    return label_data, meta_text_data
