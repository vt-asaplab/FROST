import os
import json
import random
from collections import defaultdict

input_root = 'extracted'    
output_dir = 'wiki'
target_count = 524288  

if not os.path.exists(output_dir):
    os.makedirs(output_dir)

all_shard_paths = []
for root, dirs, files in os.walk(input_root):
    for file in files:
        if file.startswith('wiki_'):
            all_shard_paths.append(os.path.join(root, file))

print(f"Found {len(all_shard_paths)} shards. Indexing all articles...")

all_articles = []
for path in all_shard_paths:
    with open(path, 'r', encoding='utf-8') as f:
        for line_num, _ in enumerate(f):
            all_articles.append((path, line_num))

total_articles = len(all_articles)
print(f"Total articles found: {total_articles}")

if total_articles < target_count:
    print(f"Only {total_articles} articles available. Adjusting to max.")
    selected_articles = all_articles
else:
    selected_articles = random.sample(all_articles, target_count)

articles = defaultdict(list)
for file_path, line_num in selected_articles:
    articles[file_path].append(line_num)

print(f"Writing {len(selected_articles)} files to '{output_dir}'...")
count = 0

for file_path, line_nums in articles.items():
    lines_to_grab = set(line_nums)
    
    with open(file_path, 'r', encoding='utf-8') as f:
        for i, line in enumerate(f):
            if i in lines_to_grab:
                data = json.loads(line)

                text = data.get('text', '').strip()
                if not text:
                    continue
                    
                filename = f"{count}.txt"
                with open(os.path.join(output_dir, filename), 'w', encoding='utf-8') as out:
                    out.write(data['text'])
                
                count += 1
                if count % 50000 == 0:
                    print(f"Progress: {count} / {len(selected_articles)}")

print(f"Done! Extracted {count} files.")
