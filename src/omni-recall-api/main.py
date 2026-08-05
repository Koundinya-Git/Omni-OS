import os
from fastapi import FastAPI, Request
import chromadb
from pydantic import BaseModel

app = FastAPI()

# Initialize ChromaDB
client = chromadb.PersistentClient(path="/home/builder/.local/share/omni/chromadb")
collection = client.get_or_create_collection(name="recall_frames")
file_collection = client.get_or_create_collection(name="file_search")

class EmbedRequest(BaseModel):
    frame_id: int
    text: str

@app.post("/api/embed")
async def embed(req: EmbedRequest):
    collection.add(
        documents=[req.text],
        metadatas=[{"frame_id": req.frame_id}],
        ids=[str(req.frame_id)]
    )
    return {"status": "ok"}

@app.get("/api/search")
async def search(q: str):
    results = collection.query(
        query_texts=[q],
        n_results=10
    )
    
    frame_ids = []
    if results['metadatas'] and len(results['metadatas']) > 0:
        for metadata in results['metadatas'][0]:
            if "frame_id" in metadata:
                frame_ids.append(metadata["frame_id"])
                
    return frame_ids

@app.post("/api/index_files")
async def index_files():
    documents_dir = os.path.expanduser("~/Documents")
    if not os.path.exists(documents_dir):
        return {"status": "error", "message": "Documents directory not found"}
        
    count = 0
    for root, dirs, files in os.walk(documents_dir):
        for file in files:
            if file.endswith(".md") or file.endswith(".txt"):
                file_path = os.path.join(root, file)
                try:
                    with open(file_path, "r", encoding="utf-8") as f:
                        content = f.read()
                    
                    file_collection.upsert(
                        documents=[content],
                        metadatas=[{"file_path": file_path}],
                        ids=[file_path]
                    )
                    count += 1
                except Exception as e:
                    pass
    return {"status": "ok", "indexed_files": count}

@app.get("/api/search_files")
async def search_files(q: str):
    results = file_collection.query(
        query_texts=[q],
        n_results=10
    )
    
    files = []
    if results['metadatas'] and len(results['metadatas']) > 0:
        for metadata in results['metadatas'][0]:
            if "file_path" in metadata:
                files.append(metadata["file_path"])
                
    return files

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="127.0.0.1", port=8000)
