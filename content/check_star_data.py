import csv

def is_column0_valid(csv_file_path):
    try:
        with open(csv_file_path, newline='') as csvfile:
            csv_reader = csv.reader(csvfile, delimiter=',')
            
            # Skip the header line
            next(csv_reader, None)
            
            for row in csv_reader:
                if len(row) >= 1:
                    # Check if the ID field (column 0) is a valid unsigned integer
                    id_string = row[0]  # Assuming "ID" is in column 0
                    id_value = int(id_string)
                else:
                    # Handle the case where there are no fields in the row
                    print("No fields found in row:", row)
                    return False

    except FileNotFoundError:
        print("Error: CSV file not found.")
        return False
    except ValueError:
        print("Invalid ID value in row:", row)
        return False

    # All IDs in column 0 are valid
    return True

# Replace 'your_csv_file.csv' with the actual path to your CSV file
csv_file_path = './hygdata_v40.csv'

if is_column0_valid(csv_file_path):
    print("All IDs in column 0 are valid.")
else:
    print("Some IDs in column 0 are invalid.")
