import json
import random

# Syllables to create pseudorandom names
syllables = ['al', 'ar', 'ba', 'ce', 'da', 'el', 'fa', 'ga', 'jo', 'ki', 'lo', 'ma', 'ne', 'or', 'pa', 'qu', 'ri', 'sa', 'te', 'vi', 'wa', 'xo', 'yi', 'zo']

def generate_star_system_name():
    name_length = random.randint(2, 3)  # Random length of the name (2 or 3 syllables)
    name = ''.join(random.choice(syllables) for _ in range(name_length)).capitalize()  # Create name from syllables
    return name

def generate_star_system():
    name = generate_star_system_name()  # Generate a pseudorandom "sounds-like" name
    distance = round(random.uniform(5, 2000), 4)  # Random distance between 5 and 20
    stellartype = random.choice(['O', 'B', 'A', 'F', 'G', 'K', 'M'])  # Random stellartype
    
    return {
        "name": name,
        "distance": distance,
        "stellartype": stellartype
    }

def generate_star_systems(num_systems):
    star_systems = [generate_star_system() for _ in range(num_systems)]
    return star_systems

# Generate 5 star systems (you can change the number as needed)
generated_star_systems = generate_star_systems(15000)

# Create a dictionary to hold the star systems
data = {"star_systems": generated_star_systems}

# Save data to a file
filename = "star_data.json"
with open(filename, "w") as file:
    json.dump(data, file, indent=2)

print(f"Generated star systems saved to '{filename}'.")

