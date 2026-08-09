package com.rubikbcd.app

import android.content.Context
import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val nameInput = findViewById<EditText>(R.id.nameInput)
        val emailInput = findViewById<EditText>(R.id.emailInput)
        val passwordInput = findViewById<EditText>(R.id.passwordInput)
        val actionButton = findViewById<Button>(R.id.actionButton)
        val statusText = findViewById<TextView>(R.id.statusText)

        val prefs = getSharedPreferences("rubikbcd", Context.MODE_PRIVATE)
        val savedEmail = prefs.getString("email", null)

        if (savedEmail != null) {
            statusText.text = "Compte local déjà enregistré : $savedEmail"
            actionButton.text = "Jouer"
        }

        actionButton.setOnClickListener {
            val name = nameInput.text.toString().trim()
            val email = emailInput.text.toString().trim()
            val password = passwordInput.text.toString().trim()

            if (name.isEmpty() || email.isEmpty() || password.isEmpty()) {
                statusText.text = "Veuillez remplir tous les champs."
                return@setOnClickListener
            }

            prefs.edit().putString("name", name).putString("email", email).putString("password", password).apply()
            statusText.text = "Compte enregistré localement. Vous pouvez maintenant utiliser l’application sur votre téléphone."
            actionButton.text = "Jouer"
        }
    }
}
