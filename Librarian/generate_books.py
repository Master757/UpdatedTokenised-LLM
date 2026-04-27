import random
#random books
def generate_books_sql(filename="books_data.sql", count=100):
    titles = ["The Shadow of {adj} {noun}", "Whispers of {noun}", "The {adj} Librarian", "Chronicles of {noun}", "{adj} Secrets"]
    adjectives = ["Ancient", "Silent", "Hidden", "Forgotten", "Midnight"]
    nouns = ["Time", "Wind", "Stars", "Stone", "Dreams"]
    authors = ["J.R.R. Scribbler", "Agatha Penman", "George R.R. Ink", "Stephen Page"]
    genres = ["Fantasy", "Mystery", "Sci-Fi", "History"]
    
    with open(filename, "w") as f:
        f.write("CREATE TABLE IF NOT EXISTS Books (\n")
        f.write("    id INTEGER PRIMARY KEY AUTOINCREMENT,\n")
        f.write("    title TEXT NOT NULL,\n")
        f.write("    author TEXT NOT NULL,\n")
        f.write("    genre TEXT NOT NULL,\n")
        f.write("    status TEXT NOT NULL\n")
        f.write(");\n;\n\n")
        
        f.write("INSERT INTO Books (title, author, genre, status) VALUES\n")
        entries = []
        for i in range(count):
            title = random.choice(titles).format(adj=random.choice(adjectives), noun=random.choice(nouns))
            entries.append(f"('{title}', '{random.choice(authors)}', '{random.choice(genres)}', 'Available')")
        f.write(",\n".join(entries) + ";\n")

if __name__ == "__main__":
    generate_books_sql()
