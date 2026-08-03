from flask import Flask, jsonify, request
from flask_cors import CORS
from flask_sqlalchemy import SQLAlchemy
from flask_jwt_extended import create_access_token, jwt_required, get_jwt_identity, JWTManager
from dotenv import load_dotenv
import bcrypt
import os
import ctypes

# 1. CONFIG
load_dotenv()
app = Flask(__name__)
CORS(app)

app.config['SQLALCHEMY_DATABASE_URI'] = f"mysql+mysqlconnector://{os.getenv('DB_USER')}:{os.getenv('DB_PASSWORD')}@{os.getenv('DB_HOST')}/{os.getenv('DB_NAME')}"
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
app.config["JWT_SECRET_KEY"] = os.getenv("JWT_SECRET_KEY")

db = SQLAlchemy(app)
jwt = JWTManager(app)

# --- CHARGEMENT DU MOTEUR C VIA CTYPES ---
lib_ext = "dll" if os.name == "nt" else "so"
lib_path = os.path.join(os.getcwd(), f"librubik.{lib_ext}")

try:
    c_lib = ctypes.CDLL(lib_path)
except OSError:
    c_lib = None
    print(f"Attention: Impossible de charger la bibliothèque C à l'emplacement {lib_path}")

class C_RubikCube(ctypes.Structure):
    _fields_ = [
        ("up", ctypes.c_char * 9),
        ("down", ctypes.c_char * 9),
        ("front", ctypes.c_char * 9),
        ("back", ctypes.c_char * 9),
        ("left", ctypes.c_char * 9),
        ("right", ctypes.c_char * 9),
    ]

# 2. MODELES
class User(db.Model):
    __tablename__ = 'users'
    id = db.Column(db.Integer, primary_key=True)
    nom = db.Column(db.String(100), nullable=False)
    email = db.Column(db.String(120), unique=True, nullable=False)
    password_hash = db.Column(db.String(255), nullable=False)
    scores = db.relationship('GameScore', backref='user', lazy=True)

    def to_dict(self):
        return {"id": self.id, "nom": self.nom, "email": self.email}

class GameScore(db.Model):
    __tablename__ = 'game_scores'
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    move_count = db.Column(db.Integer, nullable=False)
    is_solved = db.Column(db.Boolean, default=False)

    def to_dict(self):
        return {"id": self.id, "user_id": self.user_id, "move_count": self.move_count, "is_solved": self.is_solved}

with app.app_context():
    db.create_all()

# 3. ROUTES AUTH
@app.route("/api/register", methods=['POST'])
def register():
    data = request.get_json()
    if User.query.filter_by(email=data['email']).first():
        return jsonify({"error": "Email déjà utilisé"}), 409
        
    hashed_pw = bcrypt.hashpw(data['password'].encode('utf-8'), bcrypt.gensalt())
    new_user = User(nom=data['nom'], email=data['email'], password_hash=hashed_pw)
    db.session.add(new_user)
    db.session.commit()
    return jsonify({"message": "Utilisateur créé"}), 201

@app.route("/api/login", methods=['POST'])
def login():
    data = request.get_json()
    user = User.query.filter_by(email=data['email']).first()
    
    if user and bcrypt.checkpw(data['password'].encode('utf-8'), user.password_hash):
        access_token = create_access_token(identity=user.id)
        return jsonify(access_token=access_token, user=user.to_dict()), 200
    return jsonify({"error": "Email ou mot de passe incorrect"}), 401

# 4. ROUTES INTERFACE C - RUBIK'S CUBE (PROTEGEES)
@app.route("/api/cube/play", methods=['POST'])
@jwt_required()
def play_rubik_move():
    if not c_lib:
        return jsonify({"error": "Moteur C non disponible sur le serveur"}), 500
        
    data = request.get_json()
    moves = data.get('moves', [])
    
    cube = C_RubikCube()
    c_lib.reset_cube(byref_cube := ctypes.byref(cube))
    
    for mv in moves:
        c_lib.play_move(byref_cube, mv.encode('utf-8'))
        
    solved = c_lib.is_solved(byref_cube)
    
    cube_state = {
        "up": cube.up.decode('utf-8'),
        "down": cube.down.decode('utf-8'),
        "front": cube.front.decode('utf-8'),
        "back": cube.back.decode('utf-8'),
        "left": cube.left.decode('utf-8'),
        "right": cube.right.decode('utf-8'),
        "is_solved": bool(solved)
    }
    
    current_user_id = get_jwt_identity()
    new_score = GameScore(user_id=current_user_id, move_count=len(moves), is_solved=bool(solved))
    db.session.add(new_score)
    db.session.commit()

    return jsonify({"state": cube_state, "move_count": len(moves)}), 200

@app.route("/api/profil", methods=['GET'])
@jwt_required()
def profil():
    current_user_id = get_jwt_identity()
    user = User.query.get(current_user_id)
    return jsonify(user.to_dict()), 200

# 5. LANCEMENT
if __name__ == "__main__":
    app.run(debug=True, port=5000)