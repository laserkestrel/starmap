import csv

# Replace 'your_dataset.csv' with the actual filename
filename = 'hygdata_v40.csv'

# Initialize variables to store min, max, and sum of RA values
ra_min = float('inf')  # Initialize with positive infinity
ra_max = float('-inf')  # Initialize with negative infinity
ra_sum = 0
ra_count = 0

# Open the CSV file and read the RA values
with open(filename, 'r') as csvfile:
    reader = csv.DictReader(csvfile)
    for row in reader:
        # Assuming 'ra' is the column of interest
        ra_value = float(row['ra'])

        # Update min, max, sum, and count
        if ra_value < ra_min:
            ra_min = ra_value
        if ra_value > ra_max:
            ra_max = ra_value
        ra_sum += ra_value
        ra_count += 1

# Calculate the average RA value
ra_avg = ra_sum / ra_count if ra_count > 0 else 0

# Print the results
print(f"Minimum RA value: {ra_min}")
print(f"Maximum RA value: {ra_max}")
print(f"Average RA value: {ra_avg}")
